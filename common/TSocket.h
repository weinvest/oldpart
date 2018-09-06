#ifndef _OLDPART_TSOCKET_H
#define _OLDPART_TSOCKET_H
#include <memory>
#include "common/OMessage.h"
#include <boost/asio/ip/tcp.hpp>
namespace boost { namespace asio { class io_context; } }

class TSocket: public std::enable_shared_from_this<TSocket>
{
public:
    class MessageHandler
    {
    public:
        virtual ~MessageHandler() {}

        virtual void OnMessage(const std::shared_ptr<OMessage>& discover) = 0;
    };

    TSocket(const TSocket&) = delete;
    TSocket& operator= (const TSocket&) = delete;

    explicit TSocket(boost::asio::io_context& ioContext
        , std::shared_ptr<IConnectionManager> pManager
        , std::shared_ptr<MessageHandler> pHandler);

    auto& GetSocket() { return mSocket; }
    auto& GetExecutor() { return mIOContext; }
    void Send(std::shared_ptr<OMessage> pMessage);

    void Start();
    void Stop();
private:
    void DoRead();

    void DoWrite(std::shared_ptr<OMessage> pMessage);

    boost::asio::io_context& mIOContext;
    boost::asio::ip::tcp::socket mSocket;
    std::weak_ptr<IConnectionManager> mManager;
    std::shared_ptr<MessageHandler> mMessageHandler;
    std::deque<std::shared_ptr<DMessage>> mPendingMessages;
    bool mIsInSending{false};
};
#endif /* end of include guard: _OLDPART_TSOCKET_H */
