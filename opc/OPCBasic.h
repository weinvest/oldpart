#ifndef _OLDPART_OPCBASIC_H
#define _OLDPART_OPCBASIC_H
#include <memory>
#include <boost/filesystem.hpp>
#include "opp/OPCBasicProto.h"

namespace fs = boost::filesystem;

class OProtoBase;
class OMessage;
class TSocket;
class TClient;
class MessageDispatcher;
class GrepFileResponse;
class OPCBasic
{
public:
    void OnListFilesRequest(const std::shared_ptr<OMessage>& pMessage, ListFilesRequest* pListFilesRequest);
    void OnFindFilesRequest(const std::shared_ptr<OMessage>& pMessage, FindFilesRequest* pFindFilesRequest);
    void OnCatFileRequest(const std::shared_ptr<OMessage>& pMessage, CatFileRequest* pCatFileRequest);
    void OnGrepFileRequest(const std::shared_ptr<OMessage>& pMessage, GrepFileRequest* pGrepFileRequest);
    void OnGrepFilesRequest(const std::shared_ptr<OMessage>& pMessage, GrepFilesRequest* pGrepFileRequest);
private:
    FileInfo GetFileInfo(const fs::path& p);
    void GrepFile(GrepFileResponse* result
        , const std::string& p
        , const std::string& pattern
        , int32_t before
        , int32_t after);

    std::shared_ptr<TSocket> mSocket;
    std::shared_ptr<MessageDispatcher> mMessageDispatcher;
};
#endif//_OLDPART_OPCBASIC_H
