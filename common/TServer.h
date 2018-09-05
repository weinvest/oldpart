#ifndef _OLDPART_TSERVER_H
#define _OLDPART_TSERVER_H
#include <string>
#include <unordered_map>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/signal_set.hpp>
#include "common/IConnectionManager.h"

class TServer
{
public:
    TServer(const std::string& address
        , const std::string& port
        , std::shared_ptr<IConnectionManager> pConnectionManager
        , std::shared_ptr<MessageHandler> pMessageHandler
    );

    bool Send(const std::string& peer, std::shared_ptr<OMessage>& pMessage);
private:
    void WaitStop();
    void DoAccept();

    /// The io_context used to perform asynchronous operations.
    boost::asio::io_context mExecutor;

    /// The signal_set is used to register for process termination notifications.
    boost::asio::signal_set mSignals;

    /// Acceptor used to listen for incoming connections.
    boost::asio::ip::tcp::acceptor mAccetptor;
    std::unordered_map<std::string, std::shared_ptr<TSocket>> mClients;
    std::shared_ptr<IConnectionManager> mConnectionManager;
    std::shared_ptr<MessageHandler> mMessageHandler;
};
#endif /* end of include guard: _OLDPART_TSERVER_H */
