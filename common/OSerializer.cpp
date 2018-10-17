#include <zlib.h>
#include "OSerializer.h"
#include "OMessage.h"
#include "crypto/DfcCrypto.h"
#include "compress/ZLibCompressBuf.h"
#include "compress/ZLibUnCompressBuf.h"
#include "common/Utils.h"

std::function<OProtoBase*()> OSerializer::DUMP_CREATOR([]() { return (OProtoBase*)nullptr; });
OSerializer::Coro::pull_type OSerializer::Serialize(int32_t messageId, const OProtoBase& obj)
{
    return MakeMessageFromBuf(messageId, SerializeMethod::None, 0
        , [&obj](auto& sink)
          {
              obj.Write(sink, nullptr, 0);
          });
}

OSerializer::Coro::pull_type OSerializer::Serialize(int32_t messageId, const OProtoBase& obj, int8_t compressLevel)
{
    return MakeMessageFromBuf(messageId
        , SerializeMethod::Compress
        , compressLevel
        , [compressLevel, &obj, this](OProtoBase::Coro::push_type& sink)
        {
            auto write2RawBuf = [&obj](auto& sink) { obj.Write(sink, nullptr, 0); };
            CompressBuf(sink, compressLevel, write2RawBuf);
        });
}

OSerializer::Coro::pull_type OSerializer::Serialize(int32_t messageId, const OProtoBase& obj, const std::string& key)
{
    return MakeMessageFromBuf(messageId
        , SerializeMethod::Encrypt
        , 0
        , [&, this](OProtoBase::Coro::push_type& sink)
        {
            auto write2RawBuf = [&obj](auto& sink) { obj.Write(sink, nullptr, 0); };
            EncryptBuf(sink, key, write2RawBuf);
        });
}

OSerializer::Coro::pull_type OSerializer::Serialize(int32_t messageId, const OProtoBase& obj, int8_t compressLevel, const std::string& key)
{
    return MakeMessageFromBuf(messageId
        , SerializeMethod::Compress|SerializeMethod::Encrypt
        , compressLevel
        , [&, this, compressLevel](OProtoBase::Coro::push_type& sink)
        {
            auto write2RawBuf = [&obj](auto& sink1) { obj.Write(sink1, nullptr, 0); };
            auto compressBuf = [&, this](auto& sink1) { CompressBuf(sink1, compressLevel, write2RawBuf);};
            EncryptBuf(sink, key, compressBuf);
        });
}

void OSerializer::CompressBuf(OProtoBase::Coro::push_type& sink
    , int8_t level
    , std::function<void(OProtoBase::Coro::push_type&)> bufFunc)
{
    auto bufPull = OProtoBase::Coro::pull_type(boost::coroutines2::fixedsize_stack(), bufFunc);
    ZLibCompressBuf compressedBuf(make_shared_array<uint8_t>(MAX_MESSAGE_BODY_LENGTH), MAX_MESSAGE_BODY_LENGTH, level);

    for(auto body : bufPull)
    {
        auto inBuf = std::get<0>(body).get();
        auto inLen = std::get<1>(body);
        assert(inLen <= MAX_MESSAGE_BODY_LENGTH);
        while(inLen > 0)
        {
            auto compressedLen = compressedBuf.Compress(inBuf, inLen);
            assert(compressedLen > 0);
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

    if(!compressedBuf.IsEmpty())
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

OSerializer::Coro::pull_type OSerializer::MakeMessageFromBuf(int32_t messageId
    , int32_t serializeMethod
    , int8_t compressLevel
    , std::function<void(OProtoBase::Coro::push_type&)> bufFunc)
{
    return Coro::pull_type(boost::coroutines2::fixedsize_stack(),
    [&](Coro::push_type& mesgSink)
    {
        int32_t messageSequenceId = 1;
        auto bufPull = OProtoBase::Coro::pull_type(boost::coroutines2::fixedsize_stack(), bufFunc);
        std::shared_ptr<OMessage> pMessage;
        for(auto body : bufPull)
        {
            if(nullptr != pMessage)
            {
                mesgSink(pMessage);
            }

            pMessage = std::make_shared<OMessage>();
            pMessage->bodyLength = std::get<1>(body);
            pMessage->major = MESSAGE_MAJOR_VERSION;
            pMessage->minor = MESSAGE_MINOR_VERSION;
            pMessage->sequenceId = mMessageSequenceId.fetch_add(1);
            pMessage->messageId = messageId; //meesage type
            pMessage->messageSequenceId = messageSequenceId++;
            pMessage->bodySerializeMethod = serializeMethod;
            pMessage->encryptPadNum = std::get<2>(body);
            pMessage->compressLevel = compressLevel;

            pMessage->SetBody(std::get<0>(body).get());
            pMessage->SetData(std::get<0>(body));
        }

        assert(nullptr != pMessage);
        pMessage->messageSequenceId = -(messageSequenceId-1);
        mesgSink(pMessage);
    });
}


bool OSerializer::RegisteProtoCreator(int32_t messageId
    , std::function<OProtoBase*()> requestCreator
    , std::function<OProtoBase*()> responseCreator)
{
    auto insertResult = mProtoCreators.insert(std::make_pair(messageId, ProtoCreator{requestCreator, responseCreator}));
    return insertResult.second;
}

OProtoBase* OSerializer::DoCreateProto(int32_t messageId)
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

bool OSerializer::Deserialize(OProtoBase& proto, Coro::pull_type& pull, const std::string& key)
{
    auto pMessage = pull.get();
    OProtoBase::Coro::push_type sink([&proto](auto& pull)
    {
        proto.Read(pull, nullptr, 0);
    });

    do
    {
        pMessage = pull.get();
        auto pBuf = std::static_pointer_cast<uint8_t>(pMessage->GetData());
        auto bufLen = pMessage->GetBodyLength();
        if(pMessage->IsEncrypted())
        {
            auto pDecryptedBuf = make_shared_array<uint8_t>(MAX_MESSAGE_BODY_LENGTH);
            AESDecrypt((char*)pDecryptedBuf.get(), bufLen, key, (char*)pBuf.get(), bufLen, pMessage->GetPadNum());
            pBuf = pDecryptedBuf;
        }

        if(pMessage->IsCompressed())
        {
            ZLibUnCompressBuf uncompressBuf(pBuf, bufLen);
            while(!uncompressBuf.IsEmpty())
            {
                auto pUnCompressBuf = make_shared_array<uint8_t>(MAX_MESSAGE_BODY_LENGTH);
                auto compressLen = uncompressBuf.UnCompress(pUnCompressBuf.get(), MAX_MESSAGE_BODY_LENGTH);
                sink(std::make_tuple(pUnCompressBuf, compressLen, 0));
            }

            uncompressBuf.UnCompressEnd();
        }
        else
        {
            sink(std::make_tuple(pBuf, bufLen, 0));
        }

        pull();
    }while(pull);

    return true;
}
