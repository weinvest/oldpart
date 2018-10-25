#include "MessageDispatcher.h"

MessageDispatcher::MessageDispatcher()
{}

bool MessageDispatcher::RegisteHandler(int32_t msgId, Handler handler)
{
    return mHandlers.insert(std::make_pair(msgId, handler)).second;
}

bool MessageDispatcher::UnregisteHandler(int32_t msgId)
{
    return mHandlers.erase(msgId);
}

void MessageDispatcher::OnMessage(const std::shared_ptr<OMessage>& pMessage, OProtoBase* pProto)
{
    auto itHandler = mHandlers.find(pMessage->GetMessageId());
    if(itHandler != mHandlers.end())
    {
        itHandler->second(pMessage, pProto);
    }
}
