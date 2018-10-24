#ifndef _OLDPART_OPCBASIC_PROTO_H
#define _OLDPART_OPCBASIC_PROTO_H
#include "common/OEnum.h"
#include "common/OProtoBase.h"

#define OMESSAGE_RESPONSE_FIELDS\
    ((int32_t, errCode))\
    ((std::string, errMsg))

struct FileType
{
    #define FILE_TYPE_VALUES\
        ((None, 0x0))\
        ((NotFound,-1))\
        ((File, 0x1))\
        ((Directory, 0x2))\
        ((SymbolLink, 0x3))\
        ((Block, 0x4))\
        ((Character, 0x5))\
        ((Fifo, 0x6))\
        ((Socket, 0x7))\
        ((Unknown, 0x8))

    enum type
    {
        OENUM_VALUE(FILE_TYPE_VALUES)
    };

    const std::string& ToString(type v);
    bool EnumName::Parse(const std::string& value, type& v);
};

struct FileInfo: public OProtoBase
{
#define FILE_INFO_FIELDS\
    ((FileType,ftype))\
    ((std::string,name))\
    ((std::string,link2))\
    ((size_t,size))\
    ((int64_t,lastModifyTime))

    PROTO_FIELDS(FILE_INFO_FIELDS)
};

static constexpr int32_t OMESSAGE_LIST_FILES = 1025;
struct ListFilesRequest: public OProtoBase
{
#define LIST_FILES_REQUEST_FIELDS\
    ((std::string, path))

    PROTO_FIELDS(LIST_FILES_REQUEST_FIELDS)
};

struct ListFilesResponse: public OProtoBase
{
#define LIST_FILES_RESPONSE_FIELDS\
   OMESSAGE_RESPONSE_FIELDS\
   ((std::std::vector<FileInfo>, files))

   PROTO_FIELDS(LIST_FILES_RESPONSE_FIELDS)
};

static constexpr int32_t OMESSAGE_FIND_FILES = 1026;
struct FindFilesRequest: public OProtoBase
{
#define FIND_FILES_REQUEST_FIELDS\
    ((std::string, path))\
    ((std::string, pattern))\

    PROTO_FIELDS(FIND_FILES_REQUEST_FIELDS)
};

struct FindFilesResponse: public OProtoBase
{
#define FIND_FILES_RESPONSE_FIELDS\
    OMESSAGE_RESPONSE_FIELDS\
    ((std::std::vector<FileInfo>, files))
    PROTO_FIELDS(FIND_FILES_RESPONSE_FIELDS)
};

static constexpr int32_t OMESSAGE_CAT_FILE = 1027;
struct CatFileRequest: public QProtoBase
{
#define CAT_FILE_REQUEST_FIELDS\
   ((std::string, path))\
   ((int32_t, begin))\
   ((int32_t, end))

   PROTO_FIELDS(CAT_FILE_REQUEST_FIELDS)
};

struct CatFileResponse: public OProtoBase
{
#define CAT_FILE_RESPONSE_FIELDS\
   OMESSAGE_RESPONSE_FIELDS\
   ((OStream, content))

   PROTO_FIELDS(CAT_FILE_RESPONSE_FIELDS)
};

static constexpr int32_t OMESSAGE_GREP_FILE = 1028;
struct GrepFileRequest: public QProtoBase
{
#define GREP_FILE_REQUEST_FIELDS\
   ((std::string, path))\
   ((std::string, pattern))\
   ((int32_t,before))\
   ((int32_t,after))

   PROTO_FIELDS(GREP_FILE_REQUEST_FIELDS)
};

struct GrepFileResponse: public OProtoBase
{
#define GREP_FILE_RESPONSE_FIELDS\
    OMESSAGE_RESPONSE_FIELDS\
    ((std::vector<std::string>, matches))

    PROTO_FIELDS(GREP_FILE_RESPONSE_FIELDS)
};

#endif //_OLDPART_OPCBASIC_H
