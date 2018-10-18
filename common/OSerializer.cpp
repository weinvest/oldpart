#include <zlib.h>
#include <boost/crc.hpp>
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
    ZLibCompressBuf compressedBuf(make_shared_array<uint8_t>(
          OProtoSerializeHelperBase::MAX_MESSAGE_BODY_LENGTH)
        , OProtoSerializeHelperBase::MAX_MESSAGE_BODY_LENGTH
        , level);

    for(auto body : bufPull)
    {
        auto inBuf = body.buf.get();
        auto inLen = body.bufLen;
        auto isLast = body.isLast;
        assert(inLen <= OProtoSerializeHelperBase::MAX_MESSAGE_BODY_LENGTH);
        while(inLen > 0)
        {
            auto compressedLen = compressedBuf.Compress(inBuf, inLen, isLast);
            assert(compressedLen > 0);
            inBuf += compressedLen;
            inLen -= compressedLen;

            if(compressedBuf.IsFull())
            {
                compressedBuf.CompressEnd();
                sink({compressedBuf.GetOutBuf(), compressedBuf.GetOutLen(), 0, 0, isLast&&(0==inLen)});
                compressedBuf.Reset(make_shared_array<uint8_t>(OProtoSerializeHelperBase::MAX_MESSAGE_BODY_LENGTH)
                    , OProtoSerializeHelperBase::MAX_MESSAGE_BODY_LENGTH);
            }
        }
    }

    if(!compressedBuf.IsEmpty())
    {
        compressedBuf.CompressEnd();
        sink({compressedBuf.GetOutBuf(), compressedBuf.GetOutLen(), 0, 0, true});
    }
}

void OSerializer::EncryptBuf(OProtoBase::Coro::push_type& sink
    , const std::string& key
    , std::function<void(OProtoBase::Coro::push_type&)> bufFunc)
{
    auto bufPull = OProtoBase::Coro::pull_type(boost::coroutines2::fixedsize_stack(), bufFunc);
    for(auto body : bufPull)
    {
        auto pRawBuf = body.buf.get();
        auto rawBufLen = body.bufLen;
        auto pEncryptBuf = make_shared_array<uint8_t>(OProtoSerializeHelperBase::MAX_MESSAGE_BODY_LENGTH);
        int32_t encryptLen = OProtoSerializeHelperBase::MAX_MESSAGE_BODY_LENGTH;
        int32_t padNum = 0;

        int32_t checksum = ComputeChecksum(pRawBuf, rawBufLen);
        auto encryptSucc = AESEncrypt((char*)pEncryptBuf.get(), encryptLen, key
            , (char*)pRawBuf, rawBufLen, padNum);

        assert(true == encryptSucc);
        sink({pEncryptBuf, encryptLen, padNum, checksum, body.isLast});
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
            pMessage = std::make_shared<OMessage>();
            pMessage->bodyLength = body.bufLen;
            pMessage->major = MESSAGE_MAJOR_VERSION;
            pMessage->minor = MESSAGE_MINOR_VERSION;
            pMessage->sequenceId = mMessageSequenceId.fetch_add(1);
            pMessage->messageId = messageId; //meesage type
            pMessage->messageSequenceId = body.isLast ? (-messageSequenceId) : messageSequenceId;
            pMessage->bodySerializeMethod = serializeMethod;
            pMessage->encryptPadNum = body.padNum;
            pMessage->compressLevel = compressLevel;
            pMessage->checksum = body.checksum;

            pMessage->SetBody(body.buf.get());
            pMessage->SetData(body.buf);
            mesgSink(pMessage);

            ++messageSequenceId;
        }
    });
}


bool OSerializer::RegisteProtoCreator(int32_t messageId
    , std::function<OProtoBase*()> requestCreator
    , std::function<OProtoBase*()> responseCreator)
{
    auto insertResult = mProtoCreators.insert(std::make_pair(messageId
        , ProtoCreator{requestCreator, responseCreator}));
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

    ZLibUnCompressBuf uncompressBuf;
    do
    {
        pMessage = pull.get();
        auto isLast = pMessage->IsLast();
        auto pBuf = std::static_pointer_cast<uint8_t>(pMessage->GetData());
        auto bufLen = pMessage->GetBodyLength();
        if(pMessage->IsEncrypted())
        {
            auto pDecryptedBuf = make_shared_array<uint8_t>(OProtoSerializeHelperBase::MAX_MESSAGE_BODY_LENGTH);
            AESDecrypt((char*)pDecryptedBuf.get(), bufLen, key, (char*)pBuf.get(), bufLen, pMessage->GetPadNum());
            pBuf = pDecryptedBuf;
            auto checksum = ComputeChecksum(pBuf.get(), bufLen);
            if(checksum != pMessage->GetChecksum())
            {
                return false;
            }
        }

        if(pMessage->IsCompressed())
        {
            uncompressBuf.Reset(pBuf, bufLen);
            while(!uncompressBuf.IsEmpty())
            {
                auto pUnCompressBuf = make_shared_array<uint8_t>(OProtoSerializeHelperBase::MAX_MESSAGE_BODY_LENGTH);
                auto compressLen = uncompressBuf.UnCompress(pUnCompressBuf.get(), OProtoSerializeHelperBase::MAX_MESSAGE_BODY_LENGTH);
                sink({pUnCompressBuf, compressLen, 0, 0, isLast && uncompressBuf.IsEmpty()});
            }

            uncompressBuf.UnCompressEnd();
        }
        else
        {
            sink({pBuf, bufLen, 0, 0, isLast});
        }

        pull();
    }while(pull);

    return true;
}

int32_t OSerializer::ComputeChecksum(uint8_t* pBuf, int32_t bufLen)
{
    boost::crc_32_type crcComputer;
    crcComputer.process_bytes(pBuf, bufLen);
    return crcComputer.checksum();
}
