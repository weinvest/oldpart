#include "OSerializer.h"
#include "crypto/DfcCrypto.h"
template <typename T>
std::shared_ptr<T> make_shared_array(size_t size)
{
    return std::shared_ptr<T>(new T[size], std::default_delete<T[]>());
}

OMessageCoro::pull_type OSerializer::Serialize(int32_t messageId, const OProtoBase& obj)
{
    return MakeMessageFromBuf(messageId, SerializeMethod::None, obj, [](auto& sink) { obj.Write(sink, nullptr, 0); }));
}

OMessageCoro::pull_type OSerializer::Serialize(int32_t messageId, const OProtoBase& obj, int32_t compressLevel)
{
    return MakeMessageFromBuf(messageId, obj
        , SerializeMethod::Compress
        , [compressLevel](auto& sink)
        {
            CompressBuf(sink, compressLevel, [](auto& sink) { obj.Write(sink, nullptr, 0); });
        }));
}

OMessageCoro::pull_type OSerializer::Serialize(int32_t messageId, const OProtoBase& obj, const std::string& key)
{
    return MakeMessageFromBuf(messageId, obj
        , SerializeMethod::Encrypt
        , [&](auto& sink)
        {
            EncyptBuf(sink, key, [](auto& sink) { obj.Write(sink, nullptr, 0); });
        }));
}

OMessageCoro::pull_type OSerializer::Serialize(int32_t messageId, const OProtoBase& obj, int32_t compressLevel, const std::string& key)
{
    return MakeMessageFromBuf(messageId, obj
        , SerializeMethod::Compress|SerializeMethod::Encrypt
        , [&](auto& sink)
        {
            CompressBuf(sink, compressLevel, [&](auto& sink) { EncyptBuf(sink, key, [](auto& sink1) { obj.Write(sink1, nullptr, 0); }) });
        }));
}

void OSerializer::CompressBuf(OProtoBase::Coro::pull_type& sink
    , int32_t level
    , std::function<void(OProtoBase::Core::push_type&)> bufFunc)
{
    auto bufPull = OProtoBase::Core::pull_type(boost::coroutines2::fixedsize_stack(), bufFunc);
    for(auto body : bufPull)
    {

    }
}

void OSerializer::EncryptBuf(OProtoBase::Coro::pull_type& sink
    , const std::string& key
    , std::function<void(OProtoBase::Core::push_type&)> bufFunc)
{
    auto bufPull = OProtoBase::Core::pull_type(boost::coroutines2::fixedsize_stack(), bufFunc);
    for(auto body : bufPull)
    {
        auto pEncyptBuf = make_shared_array<uint8_t>(MAX_MESSAGE_BODY_LENGTH);
        int32_t encryptLen = 0;
        int32_t padNum = 0;
        AESEncrypt(pEncyptBuf.get(), encryptLen, key, body.second.get(), body.first, padNum);
        sink(std::make_pair(encryptLen, pEncryptBuf));
    }
}

OMessageCoro::pull_type OSerializer::MakeMessageFromBuf(int32_t messageId
    , int32_t serializeMethod
    , const OProtoBase& obj
    , std::function<void(OProtoBase::Core::push_type&)> bufFunc)
{
    return OMessageCoro::pull_type(boost::coroutines2::fixedsize_stack(),
    [&](OMessageCoro::push_type& mesgSink)
    {
        int32_t messageSequenceId = 0;
        auto bufPull = OProtoBase::Core::pull_type(boost::coroutines2::fixedsize_stack(), bufFunc);
        for(auto body : bufPull)
        {
            auto pMessage = std::make_shared<OMessage>();

            pMessage->length = body.first;
            pMessage->major = MESSAGE_MAJOR_VERSION;
            pMessage->minor = MESSAGE_MINOR_VERSION;
            pMessage->sequenceId = mMessageSequenceId;
            pMessage->messageId = messageId; //meesage type
            pMessage->messageSequenceId = messageSequenceId++;
            pMessage->bodySerializeMethod = serializeMethod;

            pMessage->SetBody(body.second);
            pMessage->SetData(std::make_shared<void>(body.second));

            mesgSink(pMessage);
        }
    });
}

std::shared_ptr<uint8_t> OSerializer::EnsureBuffer(OProtoBase::Coro::push_type& yield, std::shared_ptr<uint8_t> buf, int32_t& offset)
{
    auto totalLength = offset + sizeof(T);
    if(totalLength > MAX_MESSAGE_BODY_LENGTH)
    {
        yield(std::make_pair(offset, buf));
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
