#ifndef _OLDPART_OSERIALIZER_H
#define _OLDPART_OSERIALIZER_H
#include <memory>
#include <atomic>
#include <boost/coroutines2/all.hpp>
#include <<boost/call_traits.hpp>
#include "common/OProtoBase.h"

class SerializeCompressOnly{};
class SerializeEncryptOnly{};
class SerializeCompressThenEncrypt{};

class OSerializer
{
public:
    typedef std::shared_ptr<OMessage> OMessagePtr;
    typedef std::shared_ptr<>;
    typedef boost::coroutines2::coroutine<OMessagePtr> OMessageCoro;

    constexpr int32_t MAX_MESSAGE_BODY_LENGTH = 1<<21;

    OMessageCoro::pull_type Serialize(int32_t messageId, const OProtoBase& obj);
    OMessageCoro::pull_type Serialize(int32_t messageId, const OProtoBase& obj, const SerializeCompressOnly&);
    OMessageCoro::pull_type Serialize(int32_t messageId, const OProtoBase& obj, const SerializeEncryptOnly&);
    OMessageCoro::pull_type Serialize(int32_t messageId, const OProtoBase& obj, const SerializeCompressThenEncrypt&);

    template<typename T>
    int32_t WritePOD(OProtoBase::Coro::push_type& yield
        , uint8_t* buf
        , int32_t offset
        , boost::call_traits<T>::param_typeT v)
    {
        buf = EnsureBuffer(yield, buf, offset);
        memcpy(buf+offset, &v, sizeof(v));
        return offset+sizeof(v);
    }

private:
    uint8_t* EnsureBuffer(OProtoBase::Coro::push_type& yield, uint8_t* buf, int32_t& offset);
    OMessageCoro::pull_type MakeMessageFromBuf(int32_t messageId
        , int32_t serializeMethod
        , const OProtoBase& obj
        , std::function<void(OProtoBase::Core::push_type&)> bufFunc);

    void CompressBuf(OProtoBase::Coro::pull_type& sink, std::function<void(OProtoBase::Core::push_type&)> bufFunc);
    void EncryptBuf(OProtoBase::Coro::pull_type& sink, std::function<void(OProtoBase::Core::push_type&)> bufFunc);

    static constexpr int16_t MESSAGE_MAJOR_VERSION = 1;
    static constexpr int16_t MESSAGE_MINOR_VERSION = 1;
    std::atomic_int mMessageSequenceId{0};
};

#endif //_OLDPART_OSERIALIZER_H
