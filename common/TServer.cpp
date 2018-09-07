#include "TServer.h"
#include "IOContextPool.h"
TServer::TServer(std::shared_ptr<IOContextPool> pIOContextPool
    , const std::string& address
    , const std::string& port
    , std::shared_ptr<IConnectionManager> pConnectionManager
    , std::shared_ptr<TSocket::MessageHandler> pMessageHandler)
  : mIOContextPool(pIOContextPool)
  , mSignals(pIOContextPool->GetIOContext())
  , mAcceptor(pIOContextPool->GetIOContext())
  , mConnectionManager(pConnectionManager)
  , mMessageHandler(pMessageHandler)
{
    // Register to handle the signals that indicate when the server should exit.
    // It is safe to register for the same signal multiple times in a program,
    // provided all registration for the specified signal is made through Asio.
    mSignals.add(SIGINT);
    mSignals.add(SIGTERM);
    #if defined(SIGQUIT)
    mSignals.add(SIGQUIT);
    #endif // defined(SIGQUIT)

    WaitStop();

    // Open the acceptor with the option to reuse the address (i.e. SO_REUSEADDR).
    boost::asio::ip::tcp::resolver resolver(pIOContextPool->GetIOContext());
    boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve(address, port).begin();
    mAcceptor.open(endpoint.protocol());
    mAcceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
    mAcceptor.bind(endpoint);
    mAcceptor.listen();

    DoAccept();
}

bool TServer::Send(const std::string& peer, std::shared_ptr<OMessage> pMessage)
{
    auto itClient = mClients.find(peer);
    if(itClient == mClients.end())
    {
        return false;
    }

    itClient->second->Send(pMessage);
    return true;
}

void TServer::DoAccept()
{
    auto pClient = std::make_shared<TSocket>(mIOContextPool->GetIOContext(), mConnectionManager, mMessageHandler);
    auto self = shared_from_this();
    mAcceptor.async_accept(pClient->GetSocket()
      , [this, self, pClient](boost::system::error_code ec)
      {
        // Check whether the server was stopped by a signal before this
        // completion handler had a chance to run.
        if (!mAcceptor.is_open())
        {
          return;
        }

        if (!ec)
        {
	    auto remote_endpoint = pClient->GetSocket().remote_endpoint();
            auto clientName = mConnectionManager->FindInMapping(remote_endpoint.address().to_v4().to_ulong(), remote_endpoint.port());
            mClients.insert(std::make_pair(clientName, pClient));
            mConnectionManager->Start(pClient);
        }

        DoAccept();
      });
}

void TServer::WaitStop()
{
    auto self = shared_from_this();
    mSignals.async_wait(
        [this, self](boost::system::error_code /*ec*/, int /*signo*/)
        {
            // The server is stopped by cancelling all outstanding asynchronous
            // operations. Once all operations have finished the io_context::run()
            // call will exit.
            mAcceptor.close();
            mConnectionManager->StopAll();
        });
}
