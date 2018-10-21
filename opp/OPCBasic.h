#ifndef _OLDPART_OPCBASIC_H
#define _OLDPART_OPCBASIC_H
#include "common/OEnum.h"
#include "common/OProtoBase.h"

struct FileType
{
    #define FILE_TYPE_VALUES\
        ((File, 0x1))\
        ((Directory, 0x2))\
        ((SymbolLink, 0x4))

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
    ((size_t,size))\
    ((int64_t,lastModifyTime))

    PROTO_FIELDS(FILE_INFO_FIELDS)
};

static constexpr int32_t OMESSAGE_LIST_FILES = 1025;
class ListFilesRequest: public OProtoBase
{
#define LIST_FILES_REQUEST_FIELDS\
    ((std::string, path))

    PROTO_FIELDS(LIST_FILES_REQUEST_FIELDS)
};

class ListFilesResponse: public OProtoBase
{
#define LIST_FILES_RESPONSE_FIELDS\
   ((std::std::vector<FileInfo>, files))

   PROTO_FIELDS(LIST_FILES_RESPONSE_FIELDS)
};

static constexpr int32_t OMESSAGE_FIND_FILES = 1026;
class FindFilesRequest: public OProtoBase
{

};
#endif //_OLDPART_OPCBASIC_H
