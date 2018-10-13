#include <stdexcept>
#include <string>
#include "ZLibUnCompressBuf.h"

ZLibUnCompressBuf::ZLibUnCompressBuf(std::shared_ptr<uint8_t> pOutBuf, int32_t bufLen, int32_t level)
{
    Reset(pOutBuf, bufLen, level);
}

void ZLibUnCompressBuf::Reset(std::shared_ptr<uint8_t> pOutBuf, int32_t bufLen, int32_t level)
{
    mOutBuf = pOutBuf;
    mOutBufCapacity = bufLen;

    auto ret = deflateInit(&mStrm, level);
    if (ret != Z_OK)
    {
        throw std::runtime_error("ZLibUnCompressBuf deflateInit with level " + std::to_string(level) + " failed");
    }

    mStrm.zalloc = Z_NULL;
    mStrm.zfree = Z_NULL;
    mStrm.opaque = Z_NULL;

    mStrm.avail_out = bufLen;
    mStrm.next_out =  mOutBuf.get();
}

int32_t ZLibUnCompressBuf::UnCompress(uint8_t* inBuf, int32_t bufLen)
{
    mStrm.avail_in = bufLen;
    mStrm.next_in = inBuf;

    auto prevOutLen = mStrm.avail_out;
    auto ret = inflate(&mStrm, Z_NO_FLUSH);
    assert(ret != Z_STREAM_ERROR);
    return mStrm.avail_out - prevOutLen;
}
