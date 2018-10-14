#ifndef _OLDPARD_OMESSAGE_H
#define _OLDPARD_OMESSAGE_H
#include <memory>
#include <stddef.h>
#include <boost/asio/buffer.hpp>
#include <boost/endian/buffers.hpp>

struct SerializeMethod
{
    enum type
    {
        None = 0,
        Compress = 1,
        Encrypt = 2
   };
};

using bint8_t = boost::endian::big_int8_buf_at;
using bint16_t = boost::endian::big_int16_buf_at;
using bint32_t = boost::endian::big_int32_buf_at;
using bint64_t = boost::endian::big_int64_buf_at;

using ubint8_t = boost::endian::big_uint8_buf_at;
using ubint16_t = boost::endian::big_uint16_buf_at;
using ubint32_t = boost::endian::big_uint32_buf_at;
using ubint64_t = boost::endian::big_uint64_buf_at;

struct OMessage
{
    bint32_t length;
    union {
        bint32_t version;
        struct {
            bint16_t major;
            bint16_t minor;
        };
    };
    bint32_t sequenceId;
    bint32_t messageId; //meesage type
    bint32_t messageSequenceId; //一个大的Message可以被分成多个小的OMessage, 这样的OMessage从０开始编号, <0表示最后一个OMessage
    bint32_t bodySerializeMethod;

    int32_t GetSequenceId() const { return sequenceId.value(); }
    int32_t GetMessageId() const { return messageId.value(); }
    int32_t GetMessageSequenceId() const { return messageSequenceId.value(); }
    int32_t GetPadNum() const { return bodySerializeMethod.value() >> 16; }

    int32_t GetHeadLength() const { return (int32_t)offsetof(OMessage, mBody); }
    int32_t GetBodyLength() const { return length.value() - GetHeadLength(); }

    bool IsCompressed() const { return 0 != (bodySerializeMethod.value() & SerializeMethod::Compress); }
    bool IsEncrypted() const { return 0 != (bodySerializeMethod.value() & SerializeMethod::Encrypt); }

    typedef boost::asio::mutable_buffers_1 ReceiveBuffer;
    ReceiveBuffer GetReceiveBuffer();

    typedef std::vector<boost::asio::const_buffer> SendBuffer;
    SendBuffer GetSendBuffer();

    uint8_t* GetHead() { return (uint8_t*)this; }
    typedef std::shared_ptr<void> DataT;
    uint8_t* GetBody() const { return mBody; }
    void SetBody(uint8_t* body) { mBody = body; }

    void SetData(DataT pData) { mData = pData; }
    DataT GetData() { return mData; }

    uint8_t* mBody{nullptr};
    DataT mData;
};

#endif /* end of include guard: _OLDPARD_OMESSAGE_H */
