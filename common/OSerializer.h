#ifndef _OLDPART_OSERIALIZER_H
#define _OLDPART_OSERIALIZER_H
#include <memory>
#include <boost/coroutines2/all.hpp>
#include <<boost/call_traits.hpp>
class OSerializer
{
public:
    typedef std::shared_ptr<OMessage> OMessagePtr;
    typedef std::shared_ptr<>;
    typedef boost::coroutines2::coroutine<OMessagePtr> OMessageCoro;

    constexpr int32_t MAX_MESSAGE_BODY_LENGTH = 1<<21;

    template<typename T>
    OMessageCoro::pull_type Serialize(const T& obj)
    {
        return OMessageCoro::pull_type(boost::coroutines2::fixedsize_stack(),
            [&](OMessageCoro::push_type& yield)
            {
                for(OMessagePtr pMessage : obj.Write(yield))
                {
                }
            });
    }

    template<typename T>
    int32_t WritePOD(OMessageCoro::push_type& yield, uint8_t* buf, int32_t offset, boost::call_traits<T>::param_typeT v)
    {
        buf = EnsureBuffer(yield, buf, offset);
        memcpy(buf+offset, &v, sizeof(v));
        return offset+sizeof(v);
    }

private:
    uint8_t* EnsureBuffer(OMessageCoro::push_type& yield, uint8_t* buf, int32_t& offset);
};

#endif //_OLDPART_OSERIALIZER_H
