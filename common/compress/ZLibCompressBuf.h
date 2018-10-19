#ifndef _OLDPART_ZLIBCOMPRESS_BUF_H
#define _OLDPART_ZLIBCOMPRESS_BUF_H
#include <cstdint>
#include <memory>
#include <zlib.h>
class ZLibCompressBuf
{
public:
    ZLibCompressBuf(uint8_t* pOutBuf, int32_t bufLen, int32_t level);
    void Reset(uint8_t* pOutBuf, int32_t bufLen);

    int32_t Compress(uint8_t* inBuf, int32_t inLen, bool isLast);
    void CompressEnd( void );

    int32_t GetOutLen( void ) const { return mOutBufCapacity - mStrm.avail_out; }
    auto GetOutBuf( void ) { return mOutBuf; }

    bool IsFull( void ) { return GetOutLen() == mOutBufCapacity;}
    bool IsEmpty( void ) const { return 0 == GetOutLen(); }
    bool NeedMoreMemory4Tail( void ) const { return mNeedMoreMemory4Tail; }
private:
    uint8_t* mOutBuf{nullptr};
    int32_t mOutBufCapacity{0};

    z_stream mStrm;
    bool mNeedMoreMemory4Tail{false};
};

extern std::string ZLibError(int32_t ret);
#endif /* end of include guard: _OLDPART_ZLIBCOMPRESS_BUF_H */
