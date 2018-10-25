#include "OProtoResponseGuard.h"
#include "TSocket.h"
OProtoResponseGuard::~OProtoResponseGuard()
{
    mSocket->Send(-mRequest->GetMessageId(), mRequest->GetRequestId(), mResponse);
}
