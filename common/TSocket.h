#ifndef _OLDPART_TSOCKET_H
#define _OLDPART_TSOCKET_H
#include <memory>
#include <queue>
#include "common/OMessage.h"
#include <boost/asio/ip/tcp.hpp>
namespace boost { namespace asio { class io_context; } }

class IConnectionManager;
class OProtoBase;
class OSerializer;
class TSocket: public std::enable_shared_from_this<TSocket>
{
public:
    class MessageHandler
    {
    public:
        virtual ~MessageHandler() {}

        virtual bool OnMessage(const std::shared_ptr<OMessage>& pMessage) { return false; }
        virtual void OnMessage(const std::shared_ptr<OMessage>& pMessage, OProtoBase* pMessage) = 0;
    };

    TSocket(const TSocket&) = delete;
    TSocket& operator= (const TSocket&) = delete;

    explicit TSocket(boost::asio::io_context& ioContext
        , std::shared_ptr<IConnectionManager> pManager
        , std::shared_ptr<MessageHandler> pHandler);

    auto& GetSocket() { return mSocket; }
    auto& GetExecutor() { return mIOContext; }

    void SetSerializer(OSerializer* pSerializer) { mSerializer = pSerializer; }
    void SetEncryptKey(const std::string& key) { mEncryptKey = key; }
    void SetCompressLevel(int8_t compressLevel) { mCompressLevel = compressLevel; }

    void Send(std::shared_ptr<OMessage> pMessage);
    void Send(int32_t messageId, int32_t requestId, std::shared_ptr<OProtoBase> pProto);
    void Start();
    void Stop();
private:
    void DoRead();

    void DoWrite(std::shared_ptr<OMessage> pMessage);

    boost::asio::io_context& mIOContext;
    boost::asio::ip::tcp::socket mSocket;
    std::weak_ptr<IConnectionManager> mManager;
    std::shared_ptr<MessageHandler> mMessageHandler;
    std::deque<std::shared_ptr<OMessage>> mPendingMessages;
    bool mIsInSending{false};
    OSerializer* mSerializer{nullptr};
    std::string mEncryptKey;
    int8_t mCompressLevel{0};
};
#endif /* end of include guard: _OLDPART_TSOCKET_H */
