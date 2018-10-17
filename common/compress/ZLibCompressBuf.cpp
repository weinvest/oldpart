#include <stdexcept>
#include <string>
#include <cassert>
#include "ZLibCompressBuf.h"

ZLibCompressBuf::ZLibCompressBuf(std::shared_ptr<uint8_t> pOutBuf, int32_t bufLen, int32_t level)
{
    Reset(pOutBuf, bufLen, level);
}

void ZLibCompressBuf::Reset(std::shared_ptr<uint8_t> pOutBuf, int32_t bufLen, int32_t level)
{
    mOutBuf = pOutBuf;
    mOutBufCapacity = bufLen;

    mStrm.zalloc = Z_NULL;
    mStrm.zfree = Z_NULL;
    mStrm.opaque = Z_NULL;

    auto ret = deflateInit(&mStrm, level);
    if (ret != Z_OK)
    {
        throw std::runtime_error("ZLibCompressBuf deflateInit with level " + std::to_string(level) + " failed");
    }

    mStrm.avail_out = bufLen;
    mStrm.next_out =  mOutBuf.get();
}

int32_t ZLibCompressBuf::Compress(uint8_t* inBuf, int32_t bufLen)
{
    mStrm.avail_in = bufLen;
    mStrm.next_in = inBuf;

    auto ret = deflate(&mStrm, Z_NO_FLUSH);
    assert(ret != Z_STREAM_ERROR);
    return bufLen - mStrm.avail_in;
}

void ZLibCompressBuf::CompressEnd( void )
{
    deflateEnd(&mStrm);
}
