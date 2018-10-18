#ifndef _OLDPART_ZLIBUNCOMPRESS_BUF_H
#define _OLDPART_ZLIBUNCOMPRESS_BUF_H

#include <memory>
#include <cstdint>
#include <zlib.h>
class ZLibUnCompressBuf
{
public:
    ZLibUnCompressBuf();

    void Reset(std::shared_ptr<uint8_t> pInBuf, int32_t bufLen);
    int32_t UnCompress(uint8_t* outBuf, int32_t outLen);
    void UnCompressEnd();

    auto GetInLen( void ) const { return mStrm.avail_in; }

    bool IsFull( void ) { return GetInLen() == mInBufLen;}
    bool IsEmpty( void ) const { return 0 == GetInLen(); }

private:
    std::shared_ptr<uint8_t> mInBuf{nullptr};
    int32_t mInBufLen{0};

    z_stream mStrm;
};
extern std::string ZLibError(int32_t ret);
#endif //_OLDPART_ZLIBUNCOMPRESS_BUF_H
