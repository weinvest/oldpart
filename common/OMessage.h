#ifndef _OLDPARD_OMESSAGE_H
#define _OLDPARD_OMESSAGE_H
#include <memory>
#include <stddef.h>
#include <boost/asio/buffer.hpp>
struct OMessage
{
    int32_t length;
    union {
        int32_t version;
        struct {
            int16_t major;
            int16_t minor;
        };
    };
    int32_t sequenceId;
    int32_t messageId; //meesage type
    int32_t messageSequenceId; //一个大的Message可以被分成多个小的OMessage, 这样的OMessage从０开始编号, <0表示最后一个OMessage

    int32_t GetHeadLength() const { return (int32_t)offsetof(OMessage, mBody); }
    int32_t GetBodyLength() const { return length - GetHeadLength(); }

    typedef boost::asio::mutable_buffers_1 ReceiveBuffer;
    ReceiveBuffer GetReceiveBuffer();

    typedef std::vector<boost::asio::const_buffer> SendBuffer;
    SendBuffer GetSendBuffer();

    uint8_t* GetHead() { return (uint8_t*)this; }
    typedef std::shared_ptr<void> DataT;
    uint8_t* GetBody() const { return mBody; }
    void SetBody(uint8_t* body) { mBody = body; }

    void SetData(DataT pData) { mData = pData; }

    void ToHost( void );
    void ToNet( void );

    uint8_t* mBody{nullptr};
    DataT mData;
};

#endif /* end of include guard: _OLDPARD_OMESSAGE_H */
