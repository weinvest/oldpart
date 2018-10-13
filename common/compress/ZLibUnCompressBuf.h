#ifndef _OLDPART_ZLIBUNCOMPRESS_BUF_H
#define _OLDPART_ZLIBUNCOMPRESS_BUF_H

#include <memory>
#include <cstdint>
#include <zlib.h>
class ZLibUnCompressBuf
{
public:
    ZLibUnCompressBuf(std::shared_ptr<uint8_t> pOutBuf, int32_t bufLen, int32_t level);
    void Reset(std::shared_ptr<uint8_t> pOutBuf, int32_t bufLen, int32_t level);

    int32_t UnCompress(uint8_t* inBuf, int32_t inLen);

    auto GetOutLen( void ) const { return mStrm.avail_out; }
    auto GetOutBuf( void ) { return mOutBuf; }

    bool IsFull( void ) { return GetOutLen() == mOutBufCapacity;}
    bool IsEmpty( void ) const { return 0 == GetOutLen(); }

private:
    std::shared_ptr<uint8_t> mOutBuf{nullptr};
    int32_t mOutBufCapacity{0};

    z_stream mStrm;
};
#endif //_OLDPART_ZLIBUNCOMPRESS_BUF_H
