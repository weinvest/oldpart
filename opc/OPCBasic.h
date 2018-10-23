#ifndef _OLDPART_OPCBASIC_H
#define _OLDPART_OPCBASIC_H
#include <memory>
#include "opp/OPCBasicProto.h"

class OProtoBase;
class TClient;
class MessageDispatcher;
class OPCBasic
{
public:
    void OnListFilesRequest(const std::shared_ptr<OMessage>& pMessage, ListFilesRequest* pListFilesRequest);
    void OnFindFilesRequest(const std::shared_ptr<OMessage>& pMessage, FindFilesRequest* pFindFilesRequest);
    void OnCatFileRequest(const std::shared_ptr<OMessage>& pMessage, CatFileRequest* pCatFileRequest);
    void OnGrepFileRequest(const std::shared_ptr<OMessage>& pMessage, CatFileRequest* pGrepFileRequest);

private:
    std::shared_ptr<TClient> mSocket;
    std::shared_ptr<MessageDispatcher> mMessageDispatcher;
};
#endif//_OLDPART_OPCBASIC_H
