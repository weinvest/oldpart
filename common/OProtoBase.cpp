#include "OProtoBase.h"
#include "common/Utils.h"

int32_t OProtoSerializeHelperBase::MAX_MESSAGE_BODY_LENGTH = 1<<21;
std::shared_ptr<OProtoSerializeHelperBase::Buf> OProtoSerializeHelperBase::EnsureBuffer(OProtoSerializeHelperBase::Coro::push_type& yield
    , std::shared_ptr<Buf> buf
    , int32_t& offset
    , int32_t eleSize)
{
    auto totalLength = offset + eleSize;
    if(nullptr != buf && totalLength > buf->bufLen)
    {
        buf->bufLen = offset;
        yield({buf, 0, 0, false});
        buf = nullptr;
        offset = 0;
    }

    if(nullptr == buf)
    {
        buf = MakeBuffer(MAX_MESSAGE_BODY_LENGTH);
        offset = 0;
    }

    return buf;
}

std::shared_ptr<OProtoSerializeHelperBase::Buf> OProtoSerializeHelperBase::MakeBuffer(int32_t bufLen)
{
    auto pData = new uint8_t[bufLen + sizeof(Buf)];
    auto pRawBuf = new(pData)Buf{pData+sizeof(Buf), bufLen};

    return std::shared_ptr<Buf>(pRawBuf, [](Buf* p) {delete[] (uint8_t*)p;});
}

int32_t OProtoSerializeHelper<std::string,void>::Write(OProtoSerializeHelperBase::Coro::push_type& yield
    , std::shared_ptr<Buf>& buf
    , int32_t offset
    , const std::string& v)
{
    offset = OProtoSerializeHelper<int32_t,void>::Write(yield, buf, offset, v.length());
    int32_t totalLen = v.length();
    int32_t copyed = 0;
    while(totalLen > 0)
    {
        if(offset == buf->bufLen)
        {
            buf = EnsureBuffer(yield, buf, offset, 1);
        }

        auto writeLen = std::min(totalLen, buf->bufLen - offset);
        std::copy_n(v.data()+copyed, writeLen, buf->buf + offset);
        offset += writeLen;
        totalLen -= writeLen;
        copyed += writeLen;
    }

    return offset;
}

int32_t OProtoSerializeHelper<std::string,void>::Read(OProtoSerializeHelperBase::Coro::pull_type& pull
        , std::shared_ptr<Buf>& buf
        , int32_t offset
        , std::string& v)
{
    int32_t len = 0;
    offset = OProtoSerializeHelper<int32_t,void>::Read(pull, buf, offset, len);
    v.reserve(len);
    while(len > 0)
    {
        if(buf->bufLen == offset)
        {
            pull();
            auto x = pull.get();
            buf = x.buf;
            offset = 0;
        }

        auto readable = std::min(len, buf->bufLen - offset);
        v.append((char*)buf->buf+offset, readable);
        len -= readable;
        offset += readable;
    }

    return offset;
}
