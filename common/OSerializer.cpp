#include <zlib.h>
#include "OSerializer.h"
#include "OMessage.h"
#include "crypto/DfcCrypto.h"
#include "compress/ZLibCompressBuf.h"
template <typename T>
std::shared_ptr<T> make_shared_array(size_t size)
{
    return std::shared_ptr<T>(new T[size], std::default_delete<T[]>());
}

OSerializer::OMessageCoro::pull_type OSerializer::Serialize(int32_t messageId, const OProtoBase& obj)
{
    return MakeMessageFromBuf(messageId, SerializeMethod::None, [&obj](auto& sink) { obj.Write(sink, nullptr, 0); });
}

OSerializer::OMessageCoro::pull_type OSerializer::Serialize(int32_t messageId, const OProtoBase& obj, int32_t compressLevel)
{
    return MakeMessageFromBuf(messageId
        , SerializeMethod::Compress
        , [compressLevel, &obj, this](OProtoBase::Coro::push_type& sink)
        {
            CompressBuf(sink, compressLevel, [&obj](OProtoBase::Coro::push_type& sink) { obj.Write(sink, nullptr, 0); });
        });
}

OSerializer::OMessageCoro::pull_type OSerializer::Serialize(int32_t messageId, const OProtoBase& obj, const std::string& key)
{
    return MakeMessageFromBuf(messageId
        , SerializeMethod::Encrypt
        , [&, this](OProtoBase::Coro::push_type& sink)
        {
            EncryptBuf(sink, key, [&obj](auto& sink) { obj.Write(sink, nullptr, 0); });
        });
}

OSerializer::OMessageCoro::pull_type OSerializer::Serialize(int32_t messageId, const OProtoBase& obj, int32_t compressLevel, const std::string& key)
{
    return MakeMessageFromBuf(messageId
        , SerializeMethod::Compress|SerializeMethod::Encrypt
        , [&, this, compressLevel](OProtoBase::Coro::push_type& sink)
        {
            CompressBuf(sink, compressLevel, [&, this](auto& sink) { EncryptBuf(sink, key, [&obj](auto& sink1) { obj.Write(sink1, nullptr, 0); });});
        });
}

void OSerializer::CompressBuf(OProtoBase::Coro::push_type& sink
    , int32_t level
    , std::function<void(OProtoBase::Coro::push_type&)> bufFunc)
{
    auto bufPull = OProtoBase::Coro::pull_type(boost::coroutines2::fixedsize_stack(), bufFunc);
    ZLibCompressBuf compressedBuf(make_shared_array<uint8_t>(MAX_MESSAGE_BODY_LENGTH), MAX_MESSAGE_BODY_LENGTH, level);

    for(auto body : bufPull)
    {
        auto inBuf = std::get<0>(body).get();
        auto inLen = std::get<1>(body);

        while(inLen > 0)
        {
            auto compressedLen = compressedBuf.Compress(inBuf, inLen);
            inBuf += compressedLen;
            inLen -= compressedLen;

            if(compressedBuf.IsFull())
            {
                compressedBuf.CompressEnd();
                sink(std::make_tuple(compressedBuf.GetOutBuf(), compressedBuf.GetOutLen(), 0));
                compressedBuf.Reset(make_shared_array<uint8_t>(MAX_MESSAGE_BODY_LENGTH), MAX_MESSAGE_BODY_LENGTH, level);
            }
        }
    }

    if(compressedBuf.IsEmpty())
    {
        compressedBuf.CompressEnd();
        sink(std::make_tuple(compressedBuf.GetOutBuf(), compressedBuf.GetOutLen(), 0));
    }
}

void OSerializer::EncryptBuf(OProtoBase::Coro::push_type& sink
    , const std::string& key
    , std::function<void(OProtoBase::Coro::push_type&)> bufFunc)
{
    auto bufPull = OProtoBase::Coro::pull_type(boost::coroutines2::fixedsize_stack(), bufFunc);
    for(auto body : bufPull)
    {
        auto pEncryptBuf = make_shared_array<uint8_t>(MAX_MESSAGE_BODY_LENGTH);
        int32_t encryptLen = 0;
        int32_t padNum = 0;
        AESEncrypt((char*)pEncryptBuf.get(), encryptLen, key, (char*)std::get<0>(body).get(), std::get<1>(body), padNum);
        sink(std::make_tuple(pEncryptBuf, encryptLen, padNum));
    }
}

OSerializer::OMessageCoro::pull_type OSerializer::MakeMessageFromBuf(int32_t messageId
    , int32_t serializeMethod
    , std::function<void(OProtoBase::Coro::push_type&)> bufFunc)
{
    return OMessageCoro::pull_type(boost::coroutines2::fixedsize_stack(),
    [&](OMessageCoro::push_type& mesgSink)
    {
        int32_t messageSequenceId = 0;
        auto bufPull = OProtoBase::Coro::pull_type(boost::coroutines2::fixedsize_stack(), bufFunc);
        for(auto body : bufPull)
        {
            auto pMessage = std::make_shared<OMessage>();

            pMessage->length = std::get<1>(body);
            pMessage->major = MESSAGE_MAJOR_VERSION;
            pMessage->minor = MESSAGE_MINOR_VERSION;
            pMessage->sequenceId = mMessageSequenceId;
            pMessage->messageId = messageId; //meesage type
            pMessage->messageSequenceId = messageSequenceId++;
            pMessage->bodySerializeMethod = serializeMethod | std::get<2>(body);

            pMessage->SetBody(std::get<0>(body).get());
            pMessage->SetData(std::get<0>(body));

            mesgSink(pMessage);
        }
    });
}

std::shared_ptr<uint8_t> OSerializer::EnsureBuffer(OProtoBase::Coro::push_type& yield, std::shared_ptr<uint8_t> buf, int32_t& offset, int32_t eleSize)
{
    auto totalLength = offset + eleSize;
    if(totalLength > MAX_MESSAGE_BODY_LENGTH)
    {
        yield(std::make_tuple(buf, offset, 0));
        buf = nullptr;
        offset = 0;
    }

    if(nullptr == buf)
    {
        buf = make_shared_array<uint8_t>(MAX_MESSAGE_BODY_LENGTH);
        offset = 0;
    }

    return buf;
}

bool OSerializer::RegisteProtoCreator(int32_t messageId
    , std::function<OProtoBase*()> requestCreator
    , std::function<OProtoBase*()> responseCreator)
{
    auto insertResult = mProtoCreators.insert(std::make_pair(messageId, {requestCreator, responseCreator}));
    return insertResult.second;
}

OProtoBase* OSerializer::CreateProto(int32_t messageId)
{
    auto itCreator = mProtoCreators.find(messageId);
    if(itCreator == mProtoCreators.end())
    {
        return nullptr;
    }

    if(messageId > 0)
    {
        if(itCreator->second.requestCreator)
        {
            return itCreator->second.requestCreator();
        }

        return nullptr;
    }
    else
    {
        if(itCreator->second.responseCreator)
        {
            return itCreator->second.responseCreator();
        }

        return nullptr;
    }
}

bool OSerializer::Deserailize(OProtoBase*& pProto, OMessageCoro::pull_type& pull)
{
    auto pMessage = pull.get();
    pProto = CreateProto(pMessage->GetMessageId());
    if(nullptr == pProto)
    {
        return false;
    }

    do
    {
        pMessage = pull.get();
        auto pBuf = pMessage->GetData();
        auto bufLen = pMessage->GetBodyLength();
        if(pMessage->IsEncrypted())
        {
            auto pDecryptedBuf = make_shared_array<uint8_t>(MAX_MESSAGE_BODY_LENGTH);
            AESDecrypt(pDecryptedBuf.get(), bufLen, key, pBuf.get(), bufLen, pMessage->GetPadNum());
            pBuf = pDecryptedBuf;
        }

        if(pMessage->IsCompressed())
        {
            
        }
        pull();
    }while(pull);
}
