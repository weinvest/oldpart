#ifndef _OLDPART_MESSAGE_DISPATCHER_H
#define _OLDPART_MESSAGE_DISPATCHER_H
#include <functional>
#include "common/TSocket.h"
class MessageDispatcher: public TSocket::MessageHandler
{
public:
    using Handler = std::function<void(const std::shared_ptr<OMessage>&, OProtoBase*)>;
    MessageDispatcher();
    bool RegisteHandler(int32_t msgId, Handler handler);
    bool UnregisteHandler(int32_t msgId);

    void OnMessage(const std::shared_ptr<OMessage>& pMessage, OProtoBase* pMessage) override;
private:
    std::unordered_map<int32_t, Handler> mHandlers;
};

#endif //_OLDPART_MESSAGE_DISPATCHER_H
