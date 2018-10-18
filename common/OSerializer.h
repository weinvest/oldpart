#ifndef _OLDPART_OSERIALIZER_H
#define _OLDPART_OSERIALIZER_H
#include <memory>
#include <atomic>
#include <unordered_map>
#include <boost/coroutine2/all.hpp>
#include "common/OProtoBase.h"

class OMessage;
class OSerializer
{
public:
    typedef std::shared_ptr<OMessage> OMessagePtr;
    typedef boost::coroutines2::coroutine<const OMessagePtr> Coro;
    static constexpr const char* NONE_KEY = "";
    static std::function<OProtoBase*()> DUMP_CREATOR;

    Coro::pull_type Serialize(int32_t messageId, const OProtoBase& obj);
    Coro::pull_type Serialize(int32_t messageId, const OProtoBase& obj
        , int8_t compressLevel);
    Coro::pull_type Serialize(int32_t messageId, const OProtoBase& obj
        , const std::string& key);
    Coro::pull_type Serialize(int32_t messageId, const OProtoBase& obj
        , int8_t compressLevel, const std::string& key);

    bool Deserialize(OProtoBase& pProto, Coro::pull_type& pull, const std::string& key);

    bool RegisteProtoCreator(int32_t messageId
        , std::function<OProtoBase*()> requestCreator
        , std::function<OProtoBase*()> responseCreator);

    template<typename T>
    T* CreateProto(int32_t messageId)
    {
        return static_cast<T*>(DoCreateProto(messageId));
    }

    OProtoBase* DoCreateProto(int32_t messageId);
private:
    Coro::pull_type MakeMessageFromBuf(int32_t messageId
        , int32_t serializeMethod
        , int8_t compressLevel
        , std::function<void(OProtoBase::Coro::push_type&)> bufFunc);

    void CompressBuf(OProtoBase::Coro::push_type& sink, int8_t level, std::function<void(OProtoBase::Coro::push_type&)> bufFunc);
    void EncryptBuf(OProtoBase::Coro::push_type& sink, const std::string& key, std::function<void(OProtoBase::Coro::push_type&)> bufFunc);
    int32_t ComputeChecksum(uint8_t* pBuf, int32_t bufLen);

    static constexpr int16_t MESSAGE_MAJOR_VERSION = 1;
    static constexpr int16_t MESSAGE_MINOR_VERSION = 1;
    std::atomic_int mMessageSequenceId{1};

    struct ProtoCreator
    {
        std::function<OProtoBase*()> requestCreator;
        std::function<OProtoBase*()> responseCreator;
    };
    std::unordered_map<int32_t, ProtoCreator> mProtoCreators;
};

#endif //_OLDPART_OSERIALIZER_H
