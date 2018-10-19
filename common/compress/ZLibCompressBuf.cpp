#include <stdexcept>
#include <string>
#include <cassert>
#include "ZLibCompressBuf.h"

ZLibCompressBuf::ZLibCompressBuf(uint8_t* pOutBuf, int32_t bufLen, int32_t level)
:mOutBuf(pOutBuf)
,mOutBufCapacity(bufLen)
{
    mStrm.zalloc = Z_NULL;
    mStrm.zfree = Z_NULL;
    mStrm.opaque = Z_NULL;

    auto ret = deflateInit(&mStrm, level);
    if (ret != Z_OK)
    {
        throw std::runtime_error("ZLibCompressBuf deflateInit with level " + std::to_string(level) + " failed");
    }

    Reset(pOutBuf, bufLen);
}

void ZLibCompressBuf::Reset(uint8_t* pOutBuf, int32_t bufLen)
{
    mOutBuf = pOutBuf;
    mOutBufCapacity = bufLen;

    mStrm.avail_out = bufLen;
    mStrm.next_out =  mOutBuf;
}

int32_t ZLibCompressBuf::Compress(uint8_t* inBuf, int32_t bufLen, bool isLast)
{
    mStrm.avail_in = bufLen;
    mStrm.next_in = inBuf;
    auto bound = 0;
    auto flush = isLast ? Z_FINISH : Z_NO_FLUSH;

    auto ret = deflate(&mStrm, flush);
    assert(ret != Z_STREAM_ERROR);
    mNeedMoreMemory4Tail = isLast && ret == Z_OK;
    return bufLen - mStrm.avail_in;
}

void ZLibCompressBuf::CompressEnd( void )
{
    deflateEnd(&mStrm);
}

/* report a zlib or i/o error */
std::string ZLibError(int32_t ret)
{
    switch (ret)
    {
    case Z_ERRNO:
        if (ferror(stdin))
        {
            return std::string("error reading stdin");
        }
        if (ferror(stdout))
        {
            return std::string("error writing stdout");
        }
        break;
    case Z_STREAM_ERROR:
        return std::string("invalid compression level");
    case Z_DATA_ERROR:
        return std::string("invalid or incomplete deflate data");
    case Z_MEM_ERROR:
        return std::string("out of memory");
    case Z_VERSION_ERROR:
        return std::string("zlib version mismatch!");
    }

    return std::to_string(ret);
}
