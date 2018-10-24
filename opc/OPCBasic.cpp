#include "OPCBasic.h"
#include <boost/filesystem.hpp>
#include "common/OProtoResponseGuard.h"
using fs = boost::filesystem;
void OPCBasic::OnListFilesRequest(const std::shared_ptr<OMessage>& pMessage
    , ListFilesRequest* pListFilesRequest)
{
    OProtoResponseGuard responseGuard(pMessage, mSocket);
    auto pResponse = responseGuard.Create<ListFilesResponse>();
    try
    {
        fs::path p(pListFilesRequest->path);
        auto readFileInfo = [this, pResponse](fs::path& p)
        {
            pResponse->files.emplace_back();
            FileInfo& fInfo = pResponse->files.back();
            auto fStatus = fs::status(p);
            fInfo.ftype = fStatus.type;
            fInfo.name = p.name().string();
            fInfo.size = fs::file_size(p);
            fInfo.lastModifyTime = fs::last_write_time(p);

            if(fs::is_symlink(p))
            {
                fInfo.link2 = fs::read_symlink(p).string();
            }
        };

        readFileInfo(p);
        if(fs::is_directory(p))
        {
            fs::directory_iterator end;
            for(fs::directory_iterator pos(p); pos != end; ++pos)
            {
                readFileInfo(*pos);
            }
        }
    }
    catch(const fs::filesystem_error& ex)
    {
        pResponse->errCode = ex.code();
        pResponse->errMsg = ex.what();
    }
    catch(const std::exception& ex)
    {
        pResponse->errCode = -1;
        pResponse->errMsg = ex.what();
    }
}

void OPCBasic::OnFindFilesRequest(const std::shared_ptr<OMessage>& pMessage
    , FindFilesRequest* pFindFilesRequest)
{
    OProtoResponseGuard responseGuard(pMessage, mSocket);
    auto pResponse = responseGuard.Create<ListFilesResponse>();
}

void OPCBasic::OnCatFileRequest(const std::shared_ptr<OMessage>& pMessage
    , CatFileRequest* pCatFileRequest)
{

}

void OPCBasic::OnGrepFileRequest(const std::shared_ptr<OMessage>& pMessage
    , CatFileRequest* pGrepFileRequest)
{

}
