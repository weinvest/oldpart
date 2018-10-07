#include <boost/asio/write.hpp>
#include <boost/asio/read.hpp>
#include "TSocket.h"
#include "IConnectionManager.h"
TSocket::TSocket(boost::asio::io_context& ioContext
    , std::shared_ptr<IConnectionManager> pManager
    , std::shared_ptr<MessageHandler> pHandler)
    :mIOContext(ioContext)
    ,mSocket(ioContext)
    ,mManager(pManager)
    ,mMessageHandler(pHandler)
{
}

void TSocket::Send(std::shared_ptr<OMessage> pMessage)
{
    auto self = shared_from_this();
    mIOContext.post([this, self, pMessage]()
    {
        if(mPendingMessages.empty())
        {
            DoWrite(pMessage);
        }
        else
        {
            mPendingMessages.push_back(pMessage);
        }
    });
}

void TSocket::DoRead()
{
    auto self(shared_from_this());
    auto pMessage = std::make_shared<OMessage>();
    boost::asio::async_read(mSocket, boost::asio::buffer(pMessage.get(), pMessage->GetHeadLength()),
         [this, self, pMessage](boost::system::error_code ec, std::size_t bytes_transferred)
         {
            if (!ec)
            {
                if(0 == pMessage->GetBodyLength())
                {
                    DoRead();
                    mMessageHandler->OnMessage(pMessage);
                }
                else
                {
                    boost::asio::async_read(mSocket, pMessage->GetReceiveBuffer(),
                         [this, self, pMessage](boost::system::error_code ec, std::size_t bytes_transferred)
                         {
                            if (!ec)
                            {
                                DoRead();
                                mMessageHandler->OnMessage(pMessage);
                            }
                            else if (ec != boost::asio::error::operation_aborted)
                            {
                                mManager.lock()->Stop(shared_from_this());
                            }
                        });
                }
            }
            else if (ec != boost::asio::error::operation_aborted)
            {
                mManager.lock()->Stop(shared_from_this());
            }
         });
}

void TSocket::DoWrite(std::shared_ptr<OMessage> pMessage)
{
    mIsInSending = true;
    auto self(shared_from_this());

    auto bufs = std::move(pMessage->GetSendBuffer());
    boost::asio::async_write(mSocket, bufs,
     [this, self](boost::system::error_code ec, std::size_t)
     {
       if (!ec)
       {
           if(!mPendingMessages.empty())
           {
               auto pNextMessage = mPendingMessages.front();
               mPendingMessages.pop_front();
               DoWrite(pNextMessage);
           }
           else
           {
               mIsInSending = false;
           }
       }

       if (ec != boost::asio::error::operation_aborted)
       {
           mManager.lock()->Stop(shared_from_this());
       }
     });
}

void TSocket::Start()
{
    DoRead();
}

void TSocket::Stop()
{
    mSocket.close();
}
