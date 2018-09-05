#include "TSocket.h"

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

}

void TSocket::DoRead()
{
    auto self(shared_from_this());
    mSocket.async_read_some(boost::asio::buffer(buffer_),
         [this, self](boost::system::error_code ec, std::size_t bytes_transferred)
         {
            if (!ec)
            {
                request_parser::result_type result;
                std::tie(result, std::ignore) = request_parser_.parse(
                    request_, buffer_.data(), buffer_.data() + bytes_transferred);

                if (result == request_parser::good)
                {
                    request_handler_.handle_request(request_, reply_);
                    do_write();
                }
                else if (result == request_parser::bad)
                {
                    reply_ = reply::stock_reply(reply::bad_request);
                    do_write();
                }
                else
                {
                    do_read();
                }
            }
            else if (ec != boost::asio::error::operation_aborted)
            {
                mManager.lock()->Stop(shared_from_this());
            }
         });
}

void TSocket::DoWrite()
{
    auto self(shared_from_this());
    boost::asio::async_write(mSocket, reply_.to_buffers(),
     [this, self](boost::system::error_code ec, std::size_t)
     {
       if (!ec)
       {
           // Initiate graceful connection closure.
           boost::system::error_code ignored_ec;
           mSocket.shutdown(boost::asio::ip::tcp::socket::shutdown_both,
               ignored_ec);
       }

       if (ec != boost::asio::error::operation_aborted)
       {
           mManager.lock()->Stop(shared_from_this());
       }
     });
}
