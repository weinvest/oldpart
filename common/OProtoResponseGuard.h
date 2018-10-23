#ifndef _OLDPART_OPROTO_GUARD_H
#define _OLDPART_OPROTO_GUARD_H
#include <memory>

class TSocket;
class OProtoBase;
class OMessage;
class OProtoResponseGuard
{
public:
    OProtoResponseGuard(std::shared_ptr<OMessage> pRequest
        ,std::shared_ptr<TSocket> pSocket)
        :mRequest(pRequest)
        ,mSocket(pSocket)
        {}

    ~OProtoResponseGuard();

    template<typename T>
    std::shared_ptr<T> CreateResponse()
    {
        auto pResponse = std::make_shared<T>();
        mResponse = pResponse;
        return pResponse;
    }
private:
    std::shared_ptr<OMessage> mRequest;
    std::shared_ptr<OProtoBase> mResponse;
    std::shared_ptr<TSocket> mSocket;
};

#endif //_OLDPART_OPROTO_GUARD_H
