#include "OProtoResponseGuard.h"
#include "TSocket.h"
OProtoResponseGuard::~OProtoResponseGuard()
{
    mSocket->Second(-mRequest->GetMessageId(), mRequest->GetRequestId(), mResponse);
}
