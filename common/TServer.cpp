#include "TServer.h"

TTServer::TServer(TServer(const std::string& address
    , const std::string& port
    , std::shared_ptr<IConnectionManager> pConnectionManager
    , std::shared_ptr<MessageHandler> pMessageHandler)
  : mExecutor(1),
    mSignals(mExecutor),
    mAccetptor(mExecutor),
    mConnectionManager(pConnectionManager),
    mMessageHandler(pMessageHandler)
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
    boost::asio::ip::tcp::resolver resolver(mExecutor);
    boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve(address, port).begin();
    mAcceptor.open(endpoint.protocol());
    mAcceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
    mAcceptor.bind(endpoint);
    mAcceptor.listen();

    DoAccept();
}

void TServer::run()
{
    // The io_context::run() call will block until all asynchronous operations
    // have finished. While the server is running, there is always at least one
    // asynchronous operation outstanding: the asynchronous accept call waiting
    // for new incoming connections.
    mExecutor.run();
}

bool TServer::Send(const std::string& peer, std::shared_ptr<OMessage>& pMessage)
{
    auto itClient = mClients.find(peer);
    if(itClient == mClients.end())
    {
        return false;
    }

    return (*itClient)->Send(pMessage);
}

void TServer::DoAccept()
{
    mAcceptor.async_accept(
      [this](boost::system::error_code ec, boost::asio::ip::tcp::socket socket)
      {
        // Check whether the server was stopped by a signal before this
        // completion handler had a chance to run.
        if (!mAcceptor.is_open())
        {
          return;
        }

        if (!ec)
        {
            auto pClient = std::make_shared<TSocket>(std::move(socket), mConnectionManager, mMessageHandler);
            auto clientName = mConnectionManager->FindInMapping(socket.remote_endpoint().to_v4().to_ulong());
            mClients.insert(std::make_pair(clientName, pClient));
            mConnectionManager->Start(pClient);
        }

        DoAccept();
      });
}

void TServer::WaitStop()
{
  mSignals.async_wait(
      [this](boost::system::error_code /*ec*/, int /*signo*/)
      {
        // The server is stopped by cancelling all outstanding asynchronous
        // operations. Once all operations have finished the io_context::run()
        // call will exit.
        mAcceptor.close();
        mConnectionManager->StopAll();
      });
}