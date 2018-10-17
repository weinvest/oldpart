#include <cstdint>
#define BOOST_TEST_MODULE SERIALIZE_TEST
#include <boost/test/included/unit_test.hpp>
#include "common/OProtoBase.h"
#include "common/OMessage.h"
#include "common/OSerializer.h"
#define LESS_THAN_1BUF_FIELDS  ((int32_t, a1))((double, v1))((std::string, s1))
struct LessThan1Buf: public OProtoBase
{
    PROTO_FIELDS(LESS_THAN_1BUF_FIELDS);
};

PROTO_IMPLEMENTATION(LessThan1Buf, LESS_THAN_1BUF_FIELDS)

BOOST_AUTO_TEST_CASE(LessThan1Buf_None)
{
    LessThan1Buf obj;
    obj.a1 = 2;
    obj.v1 = 3.2;
    obj.s1 = "ok, i'm shgli";
    OSerializer serializer;
    auto messages = std::move(serializer.Serialize(1, obj));
    auto pMessage = messages.get();
    BOOST_TEST(1 ==  pMessage->GetMessageId());
    BOOST_TEST(false == pMessage->IsCompressed());
    BOOST_TEST(false == pMessage->IsEncrypted());
}
