#include <stdexcept>
#include <string>
#include <cassert>
#include "ZLibUnCompressBuf.h"

ZLibUnCompressBuf::ZLibUnCompressBuf(std::shared_ptr<uint8_t> pInBuf, int32_t bufLen)
    :mInBuf(pInBuf)
    ,mInBufLen(bufLen)
{
    mStrm.zalloc = Z_NULL;
    mStrm.zfree = Z_NULL;
    mStrm.opaque = Z_NULL;

    auto ret = inflateInit(&mStrm);
    if (ret != Z_OK)
    {
        throw std::runtime_error("ZLibUnCompressBuf deflateInit failed");
    }

    mStrm.avail_in = bufLen;
    mStrm.next_in = pInBuf.get();
}

void ZLibUnCompressBuf::UnCompressEnd()
{
    (void)inflateEnd(&mStrm);
}

int32_t ZLibUnCompressBuf::UnCompress(uint8_t* outBuf, int32_t outLen)
{
    mStrm.avail_out = outLen;
    mStrm.next_out = outBuf;

    auto ret = inflate(&mStrm, Z_NO_FLUSH);
    assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
    switch (ret)
    {
    case Z_NEED_DICT:
        ret = Z_DATA_ERROR;     /* and fall through */
    case Z_DATA_ERROR:
    case Z_MEM_ERROR:
        (void)inflateEnd(&mStrm);
        throw std::runtime_error("ZLibUnCompressBuf UnCompress failed, errno=" + std::to_string(ret));
    }

    return outLen - mStrm.avail_out;
}
