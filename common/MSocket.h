#ifndef _OLDPART_MSOCKET_H
#define _OLDPART_MSOCKET_H
#include <cstdint>
#include <boost/asio/ip/udp.hpp>
namespace boost { namespace asio { 
    class io_context; 
    namespace ip { 
        class address; 
    }
}
}

class Discover;
class MSocket
{
public:
    class MessageHandler
    {
    public:
        virtual ~MessageHandler() {}

        virtual void OnMessage(const Discover& discover) = 0;
        virtual bool FiltOut(const Discover& discover) = 0;
    };

    MSocket(boost::asio::io_context& io_context
        ,const boost::asio::ip::address& listen_address
        ,const boost::asio::ip::address& multicast_address
        ,int16_t multicast_port);

    ~MSocket();

    void SetMessageHandler(MessageHandler* pMessageHandler) { mMessageHandler = pMessageHandler; }

    void Send(const Discover& discover);

private:
    void DoReceive();

    boost::asio::ip::udp::socket mSocket;
    boost::asio::ip::udp::endpoint mSenderEndpoint;
    MessageHandler* mMessageHandler{nullptr};
};
#endif//_OLDPART_MSOCKET_H

