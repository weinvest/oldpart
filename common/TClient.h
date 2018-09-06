#ifndef _OLDPART_TCLIENT_H
#define _OLDPART_TCLIENT_H
#include <memory>
#include "commcon/TSocket.h"

namespace boost { namespace asio { class io_context;}}
class TClient: std::enable_shared_from_this<TClient>
{
public:
    TClient(boost::asio::io_context& ioContext, std::shared_ptr<TSocket::MessageHandler> pMessageHandler);

    void Connect(const std::string& address, const std::string& port);
    void Send(std::shared_ptr<OMessage>& pMessage);

    void Start(std::shared_ptr<TSocket> pSocket) override;
    void Stop(std::shared_ptr<TSocket> pSocket) override;
    void StopAll() override;
private:
    boost::asio::io_context& mIOContext;
    std::shared_ptr<TSocket::MessageHandler> mMessageHandler;
    std::shared_ptr<TSocket> mSocket;
};
#endif /* end of include guard: _OLDPART_TCLIENT_H */
