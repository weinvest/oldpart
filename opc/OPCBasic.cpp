#include "OPCBasic.h"
#include "common/OProtoResponseGuard.h"
#include "FileHandler.h"

#define TRY_RESPONSE(ResponseT, pMessage, pResponse)\
OProtoResponseGuard responseGuard(pMessage, mSocket);\
auto pResponse = responseGuard.CreateResponse<ResponseT>();\
try\

#define CATCH_RESPONSE(pResponse)\
catch(const fs::filesystem_error& ex)\
{\
    pResponse->errCode = ex.code().value();\
    pResponse->errMsg = ex.what();\
}\
catch(const std::exception& ex)\
{\
    pResponse->errCode = -1;\
    pResponse->errMsg = ex.what();\
}

FileInfo OPCBasic::GetFileInfo(const fs::path& p)
{
    FileInfo fInfo;
    auto fStatus = fs::status(p);
    fInfo.ftype = (FileType::type)fStatus.type();
    fInfo.name = p.string();
    fInfo.size = fs::file_size(p);
    fInfo.lastModifyTime = fs::last_write_time(p);

    if(fs::is_symlink(p))
    {
        fInfo.link2 = fs::read_symlink(p).string();
    }

    return fInfo;
}

void OPCBasic::OnListFilesRequest(const std::shared_ptr<OMessage>& pMessage
    , ListFilesRequest* pListFilesRequest)
{
    TRY_RESPONSE(ListFilesResponse, pMessage, pResponse)
    {
        fs::path p(pListFilesRequest->path);
        auto fInfo = GetFileInfo(p);
        pResponse->files.push_back(fInfo);
        if(fs::is_directory(p))
        {
            fs::directory_iterator end;
            for(fs::directory_iterator pos(p); pos != end; ++pos)
            {
                fInfo = GetFileInfo(*pos);
                pResponse->files.push_back(fInfo);
            }
        }
    }
    CATCH_RESPONSE(pResponse)
}

void OPCBasic::OnFindFilesRequest(const std::shared_ptr<OMessage>& pMessage
    , FindFilesRequest* pFindFilesRequest)
{
    TRY_RESPONSE(FindFilesResponse, pMessage, pResponse)
    {
        PathVec files;
        FindFiles(pFindFilesRequest->path, pFindFilesRequest->pattern, files);
        for(auto& p : files)
        {
            auto fInfo = GetFileInfo(p);
            pResponse->files.push_back(fInfo);
        }
    }
    CATCH_RESPONSE(pResponse)
}

void OPCBasic::OnCatFileRequest(const std::shared_ptr<OMessage>& pMessage
    , CatFileRequest* pCatFileRequest)
{
    TRY_RESPONSE(CatFileResponse, pMessage, pResponse)
    {

    }
    CATCH_RESPONSE(pResponse)
}

void OPCBasic::OnGrepFileRequest(const std::shared_ptr<OMessage>& pMessage
    , CatFileRequest* pGrepFileRequest)
{
    TRY_RESPONSE(GrepFileResponse, pMessage, pResponse)
    {
        std::fstream inFile(pGrepFileRequest->path, std::ios::binary|std::ios::in);
        std::string line;
        while(std::getline(inFile, line))
        {
            
        }
    }
    CATCH_RESPONSE(pResponse)
}
