#ifndef _OLDPART_OSERIALIZER_H
#define _OLDPART_OSERIALIZER_H
#include <memory>
#include <atomic>
#include <boost/coroutine2/all.hpp>
#include <boost/call_traits.hpp>
#include "common/OProtoBase.h"

class OMessage;
class OSerializer
{
public:
    typedef std::shared_ptr<OMessage> OMessagePtr;
    typedef boost::coroutines2::coroutine<OMessagePtr> OMessageCoro;

    static constexpr int32_t MAX_MESSAGE_BODY_LENGTH = 1<<21;

    OMessageCoro::pull_type Serialize(int32_t messageId, const OProtoBase& obj);
    OMessageCoro::pull_type Serialize(int32_t messageId, const OProtoBase& obj
        , int32_t compressLevel);
    OMessageCoro::pull_type Serialize(int32_t messageId, const OProtoBase& obj
        , const std::string& key);
    OMessageCoro::pull_type Serialize(int32_t messageId, const OProtoBase& obj
        , int32_t compressLevel, const std::string& key);

    template<typename T>
    int32_t WritePOD(OProtoBase::Coro::push_type& yield
        , std::shared_ptr<uint8_t> buf
        , int32_t offset
        , typename boost::call_traits<T>::param_type v)
    {
        buf = EnsureBuffer(yield, buf, offset, sizeof(T));
        memcpy(buf.get()+offset, &v, sizeof(v));
        return offset+sizeof(v);
    }

private:
    std::shared_ptr<uint8_t> EnsureBuffer(OProtoBase::Coro::push_type& yield, std::shared_ptr<uint8_t> buf, int32_t& offset, int32_t eleSize);
    OMessageCoro::pull_type MakeMessageFromBuf(int32_t messageId
        , int32_t serializeMethod
        , std::function<void(OProtoBase::Coro::push_type&)> bufFunc);

    void CompressBuf(OProtoBase::Coro::push_type& sink, int32_t level, std::function<void(OProtoBase::Coro::push_type&)> bufFunc);
    void EncryptBuf(OProtoBase::Coro::push_type& sink, const std::string& key, std::function<void(OProtoBase::Coro::push_type&)> bufFunc);

    static constexpr int16_t MESSAGE_MAJOR_VERSION = 1;
    static constexpr int16_t MESSAGE_MINOR_VERSION = 1;
    std::atomic_int mMessageSequenceId{0};
};

#endif //_OLDPART_OSERIALIZER_H
