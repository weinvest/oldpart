#include <boost/asio/connect.hpp>
#include "TClient.h"

TClient::TClient(boost::asio::io_context& ioContext
    , std::shared_ptr<TSocket::MessageHandler> pMessageHandler)
    :mIOContext(ioContext)
    ,mMessageHandler(pMessageHandler)
{
}

void TClient::Connect(const std::string& address, const std::string& port)
{
    boost::asio::ip::tcp::resolver resolver(mIOContext);
    boost::asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(address, port);
    mSocket = std::make_shared<TSocket>(mIOContext, shared_from_this(), mMessageHandler);
    boost::asio::connect(mSocket->GetSocket(), endpoints);
}

void TClient::Send(std::shared_ptr<OMessage>& pMessage)
{
    mSocket->Send(pMessage);
}

void TClient::Start(std::shared_ptr<TSocket> pSocket)
{
    pSocket->Start();
}

void TClient::Stop(std::shared_ptr<TSocket> pSocket)
{
    pSocket->Stop();
}

void TClient::StopAll()
{
    mSocket->Stop();
}
