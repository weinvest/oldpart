#include "OPCBasic.h"
#include <boost/circular_buffer.hpp>
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

struct GrepTrace
{
    int32_t lineNo{-1};
    bool matched{false};
    std::string line;
};

void OPCBasic::OnGrepFileRequest(const std::shared_ptr<OMessage>& pMessage
    , GrepFileRequest* pGrepFileRequest)
{
    TRY_RESPONSE(GrepFileResponse, pMessage, pResponse)
    {
        if(pGrepFileRequest->before < 0)
        {
            throw std::invalid_argument("GrepFileRequest::before should greatequal 0");
        }

        if(pGrepFileRequest->after < 0)
        {
           throw  std::invalid_argument("GrepFileRequest::after should greatequal 0");
        }

        std::fstream inFile(pGrepFileRequest->path, std::ios::binary|std::ios::in);
        auto pattern = re::sregex::compile(pGrepFileRequest->pattern);
        int32_t lineNo = 1;
        int32_t matchIndex = -1;
        int32_t afterCount = pGrepFileRequest->after;
        boost::circular_buffer<GrepTrace> beforeBuffer(pGrepFileRequest->before+1);
        beforeBuffer.push_back(GrepTrace());

        auto* pGrepTrace = &beforeBuffer.back();
        while(std::getline(inFile, pGrepTrace->line))
        {
            pGrepTrace->lineNo = lineNo++;

            re::sregex_token_iterator begin(pGrepTrace->line.begin(), pGrepTrace->line.end(), pattern);
            decltype(begin) end;
            if(begin != end)
            {
                pGrepTrace->matched = true;
                pResponse->matches.emplace_back();
                auto& match = pResponse->matches.back();
                match.lineNo = pGrepTrace->lineNo;
                ++matchIndex;
                for(auto itSubMatch = begin; itSubMatch != end; ++itSubMatch)
                {
                    match.subMatches.emplace_back();
                    auto& subMatch = match.subMatches.back();
                    subMatch.from = (itSubMatch)->first - pGrepTrace->line.begin();
                    subMatch.to = (itSubMatch)->second - pGrepTrace->line.begin();;
                }

                while(!beforeBuffer.empty())
                {
                    auto& topTrace = beforeBuffer.front();
                    pResponse->selectedLines.emplace_back();
                    auto& selectedLine = pResponse->selectedLines.back();
                    selectedLine.lineNo = topTrace.lineNo;
                    selectedLine.matchIndex = matchIndex;
                    selectedLine.line = std::move(topTrace.line);
                    beforeBuffer.pop_front();
                }

                afterCount = pGrepFileRequest->after;
            }
            else if(afterCount > 0)
            {
                afterCount--;
                auto& topTrace = beforeBuffer.front();
                pResponse->selectedLines.emplace_back();
                auto& selectedLine = pResponse->selectedLines.back();
                selectedLine.lineNo = topTrace.lineNo;
                selectedLine.matchIndex = matchIndex;
                selectedLine.line = std::move(topTrace.line);
                beforeBuffer.pop_front();
            }

            beforeBuffer.push_back(GrepTrace());
            pGrepTrace = &beforeBuffer.back();
        }
    }
    CATCH_RESPONSE(pResponse)
}
