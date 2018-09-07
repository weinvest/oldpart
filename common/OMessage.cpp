#include "common/OMessage.h"
void OMessage::ToHost( void )
{

}

void OMessage::ToNet( void )
{

}

OMessage::ReceiveBuffer OMessage::GetReceiveBuffer()
{
    auto pBodyBuf = std::make_shared<std::string>();
    pBodyBuf->resize(GetBodyLength());
    //for keep the reference the buffer pBodyBuf
    SetData(pBodyBuf);
    mBody = const_cast<uint8_t*>((uint8_t*)(pBodyBuf->c_str()));
    //EnsureBuffers();
    return boost::asio::buffer(boost::asio::mutable_buffer(mBody, GetBodyLength()));
}

OMessage::SendBuffer OMessage::GetSendBuffer()
{
    SendBuffer bufs;
    bufs.push_back(boost::asio::const_buffer(GetHead(), GetHeadLength()));
    if(0 != GetBodyLength())
    {
        bufs.push_back(boost::asio::const_buffer(GetBody(), GetBodyLength()));
    }

    return bufs;
}
