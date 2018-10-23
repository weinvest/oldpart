#include "OPCBasic.h"
#include <boost/filesystem.hpp>
#include "common/OProtoResponseGuard.h"
using fs = boost::filesystem;
void OPCBasic::OnListFilesRequest(const std::shared_ptr<OMessage>& pMessage, ListFilesRequest* pListFilesRequest)
{
    OProtoResponseGuard responseGuard(pMessage, mSocket);
    auto pResponse = responseGuard.Create<ListFilesResponse>();
    if(!fs::exists(pListFilesRequest->path))
    {

    }
}

void OPCBasic::OnFindFilesRequest(const std::shared_ptr<OMessage>& pMessage, FindFilesRequest* pFindFilesRequest)
{

}

void OPCBasic::OnCatFileRequest(const std::shared_ptr<OMessage>& pMessage, CatFileRequest* pCatFileRequest)
{

}

void OPCBasic::OnGrepFileRequest(const std::shared_ptr<OMessage>& pMessage, CatFileRequest* pGrepFileRequest)
{

}
