#include "OSerializer.h"
OMessageCoro::pull_type OSerializer::Serialize(const OProtoBase& obj)
{
    return OMessageCoro::pull_type(boost::coroutines2::fixedsize_stack(),
        [&](OMessageCoro::push_type& yield)
        {
            for(OMessagePtr pMessage : obj.Write(yield))
            {
            }
        });
}

uint8_t* OSerializer::EnsureBuffer(OMessageCoro::push_type& yield, uint8_t* buf, int32_t& offset)
{
    auto totalLength = offset + sizeof(T);
    if(totalLength > MAX_MESSAGE_BODY_LENGTH)
    {
	yield(buf);
	buf = nullptr;
	offset = 0;
    }

    if(nullptr == buf)
    {
	buf = ;
    	offset = 0;
    }

    return buf;
}
