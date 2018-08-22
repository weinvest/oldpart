#include "MSocket.h"
MSocket::MSocket(boost::asio::io_context& io_context
    ,const boost::asio::ip::address& listen_address
    ,const boost::asio::ip::address& multicast_address
    ,int16_t multicast_port)
:mSocket(io_context)
,mSenderEndpoint(multicast_address, multicast_port)
{
    // Create the socket so that multiple may be bound to the same address.
    boost::asio::ip::udp::endpoint listen_endpoint(
        listen_address, multicast_port);
    mSocket.open(listen_endpoint.protocol());
    mSocket.set_option(boost::asio::ip::udp::socket::reuse_address(true));
    mSocket.bind(listen_endpoint);

    // Join the multicast group.
    mSocket.set_option(
        boost::asio::ip::multicast::join_group(multicast_address));

    DoReceive();
}

void MSocket::Send(const Discover& discover)
{
    mSocket.async_send_to(
        boost::asio::buffer(&discover, sizeof(discover)), mSenderEndpoint,
        [this](boost::system::error_code ec, std::size_t /*length*/)
        {
            
        });
}

void MSocket::DoReceive()
{
  Discover discover;
  mSocket.async_receive_from(
      boost::asio::buffer(&discover, sizeof(discover)), mSenderEndpoint,
      [this, &discover](boost::system::error_code ec, std::size_t length)
      {
        if (!ec)
        {
          if(length == sizeof(discover) && !mMessageHandler->FiltOut(discover))
          {
              mMessageHandler->OnMessage(discover);
          }
          DoReceive();
        }
      });
}

MSocket::~MSocket()
{}

