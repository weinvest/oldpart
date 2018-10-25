#include "IOStream.h"

int32_t IOStream::Write(Coro::push_type& yield, std::shared_ptr<Buf>& buf, int32_t offset) const
{
    offset = OProtoSerializeHelper<int32_t,void>::Write(yield, buf, offset, this->size);
    int32_t totalLen = this->size;
    int32_t copyed = 0;
    while(totalLen > 0)
    {
        if(offset == buf->bufLen)
        {
            buf = OProtoSerializeHelperBase::EnsureBuffer(yield, buf, offset, 1);
        }

        auto writeLen = std::min(totalLen, buf->bufLen - offset);
        this->stream->read((char*)buf->buf + offset, writeLen);
        offset += writeLen;
        totalLen -= writeLen;
        copyed += writeLen;
    }

    return offset;
}

int32_t IOStream::Read(Coro::pull_type& pull, std::shared_ptr<Buf>& buf, int32_t offset)
{
    int32_t len = 0;
    offset = OProtoSerializeHelper<int32_t,void>::Read(pull, buf, offset, len);
    this->size = len;
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
        this->stream->write((char*)buf->buf + offset, readable);
        len -= readable;
        offset += readable;
    }

    return offset;
}
