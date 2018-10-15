#include "OProtoBase.h"

std::shared_ptr<uint8_t> OProtoBase::EnsureBuffer(Coro::push_type& yield
    , std::shared_ptr<uint8_t>& buf
    , int32_t& offset
    , int32_t eleSize)
{
    auto totalLength = offset + eleSize;
    if(totalLength > MAX_MESSAGE_BODY_LENGTH)
    {
        yield(std::make_tuple(buf, offset, 0));
        buf = nullptr;
        offset = 0;
    }

    if(nullptr == buf)
    {
        buf = make_shared_array<uint8_t>(MAX_MESSAGE_BODY_LENGTH);
        offset = 0;
    }

    return buf;
}

int32_t OProtoBase::WriteFild(Coro::push_type& yield
    , std::shared_ptr<uint8_t>& buf
    , int32_t offset
    , const std::string& v)
{
    offset = WriteFild<int32_t>(yield, buf, offset, v.length());
    auto totalLen = v.length();
    while(totalLen > 0)
    {
        if(offset == MAX_MESSAGE_BODY_LENGTH)
        {
            buf = EnsureBuffer(yield, buf, offset, 1);
        }

        auto writeLen = std::min(totalLen, MAX_MESSAGE_BODY_LENGTH - offset);
        memcpy(buf.get() + offset, v.data(), writeLen);
        offset += writeLen;
        totalLen -= writeLen;
    }

    return offset;
}

int32_t OProtoBase::ReadField(Coro::pull_type& pull
    , std::shared_ptr<uint8_t>& buf
    , int32_t offset
    , std::string& v)
{
    int32_t len = 0;
    offset = ReadField<int32_t>(pull, buf, offset, len);
    v.resize(len);
    while(len > 0)
    {
        if(MAX_MESSAGE_BODY_LENGTH == offset)
        {
            auto x = pull.get();
            buf = std::get<0>(x);
            offset = 0;
        }

        auto readable = std::min(len, MAX_MESSAGE_BODY_LENGTH - offset);
        memcpy(v.data() + v.length() - len, buf.get(), readable);
        len -= readable;
        offset += readable;
    }

    return offset;
}
