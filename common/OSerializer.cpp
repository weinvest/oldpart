#include "OSerializer.h"
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

