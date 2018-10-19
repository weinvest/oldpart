#include "OProtoBase.h"
#include "common/Utils.h"

int32_t OProtoSerializeHelperBase::MAX_MESSAGE_BODY_LENGTH = 1<<21;
std::shared_ptr<uint8_t> OProtoSerializeHelperBase::EnsureBuffer(OProtoSerializeHelperBase::Coro::push_type& yield
    , std::shared_ptr<uint8_t>& buf
    , int32_t& offset
    , int32_t eleSize)
{
    auto totalLength = offset + eleSize;
    if(totalLength > MAX_MESSAGE_BODY_LENGTH)
    {
        yield({buf, offset, 0, 0, false});
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

int32_t OProtoSerializeHelper<std::string>::Write(OProtoSerializeHelperBase::Coro::push_type& yield
    , std::shared_ptr<uint8_t>& buf
    , int32_t offset
    , const std::string& v)
{
    offset = OProtoSerializeHelper<int32_t>::Write(yield, buf, offset, v.length());
    int32_t totalLen = v.length();
    int32_t copyed = 0;
    while(totalLen > 0)
    {
        if(offset == MAX_MESSAGE_BODY_LENGTH)
        {
            buf = EnsureBuffer(yield, buf, offset, 1);
        }

        auto writeLen = std::min(totalLen, MAX_MESSAGE_BODY_LENGTH - offset);
        std::copy_n(v.data()+copyed, writeLen, buf.get() + offset);
        offset += writeLen;
        totalLen -= writeLen;
        copyed += writeLen;
    }

    return offset;
}

int32_t OProtoSerializeHelper<std::string>::Read(OProtoSerializeHelperBase::Coro::pull_type& pull
        , std::shared_ptr<uint8_t>& buf
        , int32_t offset
        , std::string& v)
{
    int32_t len = 0;
    offset = OProtoSerializeHelper<int32_t>::Read(pull, buf, offset, len);
    v.reserve(len);
    while(len > 0)
    {
        if(MAX_MESSAGE_BODY_LENGTH == offset)
        {
            pull();
            auto x = pull.get();
            buf = x.buf;
            offset = 0;
        }

        auto readable = std::min(len, MAX_MESSAGE_BODY_LENGTH - offset);
        v.append((char*)buf.get()+offset, readable);
        len -= readable;
        offset += readable;
    }

    return offset;
}
