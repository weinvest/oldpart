#include <cstdint>
#define BOOST_TEST_MODULE SERIALIZE_TEST
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
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
    serializer.RegisteProtoCreator(1
        , [](){ return new LessThan1Buf(); }
        , OSerializer::DUMP_CREATOR);

    auto messages = std::move(serializer.Serialize(1, obj));
    auto pMessage = messages.get();

    BOOST_TEST(1 ==  pMessage->GetSequenceId());
    BOOST_TEST(1 ==  pMessage->GetMessageId());
    BOOST_TEST(-1 == pMessage->GetMessageSequenceId()); //only one message
    BOOST_TEST(0 == pMessage->GetPadNum());
    BOOST_TEST(0 == pMessage->GetCompressLevel());
    BOOST_TEST((sizeof(obj.a1) + sizeof(obj.v1) + sizeof(int32_t) + obj.s1.length()) == pMessage->GetBodyLength());
    BOOST_TEST(false == pMessage->IsCompressed());
    BOOST_TEST(false == pMessage->IsEncrypted());

    LessThan1Buf* deObj = serializer.CreateProto<LessThan1Buf>(pMessage->GetMessageId());
    BOOST_TEST(true == serializer.Deserialize(*deObj, messages, OSerializer::NONE_KEY));
    BOOST_TEST(deObj->a1 == obj.a1);
    BOOST_TEST(deObj->v1 == obj.v1);
    BOOST_TEST(deObj->s1 == obj.s1);
}

BOOST_AUTO_TEST_CASE(LessThan1Buf_Compress)
{
    LessThan1Buf obj;
    obj.a1 = 2;
    obj.v1 = 3.2;
    obj.s1 = "ok, i'm shgli";
    OSerializer serializer;
    serializer.RegisteProtoCreator(1
        , [](){ return new LessThan1Buf(); }
        , OSerializer::DUMP_CREATOR);

    auto compressLevel = 2;
    auto messages = std::move(serializer.Serialize(1, obj, compressLevel));
    auto pMessage = messages.get();

    BOOST_TEST(1 ==  pMessage->GetSequenceId());
    BOOST_TEST(1 ==  pMessage->GetMessageId());
    BOOST_TEST(-1 == pMessage->GetMessageSequenceId()); //only one message
    BOOST_TEST(0 == pMessage->GetPadNum());
    BOOST_TEST(compressLevel == pMessage->GetCompressLevel());
    BOOST_TEST(true == pMessage->IsCompressed());
    BOOST_TEST(false == pMessage->IsEncrypted());

    LessThan1Buf* deObj = serializer.CreateProto<LessThan1Buf>(pMessage->GetMessageId());
    BOOST_TEST(true == serializer.Deserialize(*deObj, messages, OSerializer::NONE_KEY));
    BOOST_TEST(deObj->a1 == obj.a1);
    BOOST_TEST(deObj->v1 == obj.v1);
    BOOST_TEST(deObj->s1 == obj.s1);
}

BOOST_AUTO_TEST_CASE(LessThan1Buf_Encrypt)
{
    LessThan1Buf obj;
    obj.a1 = 2;
    obj.v1 = 3.2;
    obj.s1 = "ok, i'm shgli";
    OSerializer serializer;
    serializer.RegisteProtoCreator(1
        , [](){ return new LessThan1Buf(); }
        , OSerializer::DUMP_CREATOR);

    auto key("test key");
    auto messages = std::move(serializer.Serialize(1, obj, key));
    auto pMessage = messages.get();

    BOOST_TEST(1 ==  pMessage->GetSequenceId());
    BOOST_TEST(1 ==  pMessage->GetMessageId());
    BOOST_TEST(-1 == pMessage->GetMessageSequenceId()); //only one message
    // BOOST_TEST(0 == pMessage->GetPadNum());
    BOOST_TEST(0 == pMessage->GetCompressLevel());
    BOOST_TEST(false == pMessage->IsCompressed());
    BOOST_TEST(true == pMessage->IsEncrypted());

    LessThan1Buf* deObj = serializer.CreateProto<LessThan1Buf>(pMessage->GetMessageId());
    BOOST_TEST(true == serializer.Deserialize(*deObj, messages, key));
    BOOST_TEST(deObj->a1 == obj.a1);
    BOOST_TEST(deObj->v1 == obj.v1);
    BOOST_TEST(deObj->s1 == obj.s1);

    auto errKey("errKey");
    LessThan1Buf* deObjErr = serializer.CreateProto<LessThan1Buf>(pMessage->GetMessageId());
    BOOST_TEST(false == serializer.Deserialize(*deObj, messages, errKey));
    BOOST_TEST(deObjErr->a1 != obj.a1);
    BOOST_TEST(deObjErr->v1 != obj.v1);
    BOOST_TEST(deObjErr->s1 != obj.s1);
}

BOOST_AUTO_TEST_CASE(LessThan1Buf_Compress_And_Encrypt)
{
    LessThan1Buf obj;
    obj.a1 = 2;
    obj.v1 = 3.2;
    obj.s1 = "ok, i'm shgli";
    OSerializer serializer;
    serializer.RegisteProtoCreator(1
        , [](){ return new LessThan1Buf(); }
        , OSerializer::DUMP_CREATOR);

    auto key("test key");
    auto compressLevel = 2;
    auto messages = std::move(serializer.Serialize(1, obj, compressLevel, key));
    auto pMessage = messages.get();

    BOOST_TEST(1 ==  pMessage->GetSequenceId());
    BOOST_TEST(1 ==  pMessage->GetMessageId());
    BOOST_TEST(-1 == pMessage->GetMessageSequenceId()); //only one message
    // BOOST_TEST(0 == pMessage->GetPadNum());
    BOOST_TEST(compressLevel == pMessage->GetCompressLevel());
    BOOST_TEST(true == pMessage->IsCompressed());
    BOOST_TEST(true == pMessage->IsEncrypted());

    LessThan1Buf* deObj = serializer.CreateProto<LessThan1Buf>(pMessage->GetMessageId());
    BOOST_TEST(true == serializer.Deserialize(*deObj, messages, key));
    BOOST_TEST(deObj->a1 == obj.a1);
    BOOST_TEST(deObj->v1 == obj.v1);
    BOOST_TEST(deObj->s1 == obj.s1);

    auto errKey("errKey");
    LessThan1Buf* deObjErr = serializer.CreateProto<LessThan1Buf>(pMessage->GetMessageId());
    BOOST_TEST(false == serializer.Deserialize(*deObj, messages, errKey));
}
