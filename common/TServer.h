#ifndef _OLDPART_TSERVER_H
#define _OLDPART_TSERVER_H
#include <string>
#include <unordered_map>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/signal_set.hpp>
#include "common/IConnectionManager.h"
#include "common/TSocket.h"

class IOContextPool;
class TServer: std::enable_shared_from_this<TServer>
{
public:
    TServer(std::shared_ptr<IOContextPool> pIOContextPool
        , const std::string& address
        , const std::string& port
        , std::shared_ptr<IConnectionManager> pConnectionManager
        , std::shared_ptr<TSocket::MessageHandler> pMessageHandler
    );

    bool Send(const std::string& peer, std::shared_ptr<OMessage> pMessage);
private:
    void WaitStop();
    void DoAccept();

    std::shared_ptr<IOContextPool> mIOContextPool;

    /// The signal_set is used to register for process termination notifications.
    boost::asio::signal_set mSignals;

    /// Acceptor used to listen for incoming connections.
    boost::asio::ip::tcp::acceptor mAcceptor;
    std::unordered_map<std::string, std::shared_ptr<TSocket>> mClients;
    std::shared_ptr<IConnectionManager> mConnectionManager;
    std::shared_ptr<TSocket::MessageHandler> mMessageHandler;
};
#endif /* end of include guard: _OLDPART_TSERVER_H */
