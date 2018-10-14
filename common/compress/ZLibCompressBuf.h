#ifndef _OLDPART_ZLIBCOMPRESS_BUF_H
#define _OLDPART_ZLIBCOMPRESS_BUF_H
#include <cstdint>
#include <memory>
#include <zlib.h>
class ZLibCompressBuf
{
public:
    ZLibCompressBuf(std::shared_ptr<uint8_t> pOutBuf, int32_t bufLen, int32_t level);
    void Reset(std::shared_ptr<uint8_t> pOutBuf, int32_t bufLen, int32_t level);

    int32_t Compress(uint8_t* inBuf, int32_t inLen);
    void CompressEnd( void );

    auto GetOutLen( void ) const { return mStrm.avail_out; }
    auto GetOutBuf( void ) { return mOutBuf; }

    bool IsFull( void ) { return GetOutLen() == mOutBufCapacity;}
    bool IsEmpty( void ) const { return 0 == GetOutLen(); }

private:
    std::shared_ptr<uint8_t> mOutBuf{nullptr};
    int32_t mOutBufCapacity{0};

    z_stream mStrm;
};

#endif /* end of include guard: _OLDPART_ZLIBCOMPRESS_BUF_H */