#include <cstdint>
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include "common/OProtoBase.h"
#include "common/OMessage.h"
#include "common/OSerializer.h"
#define NEST_OBJECT_FIELDS ((uint8_t, n1))((std::string, n2))((double, n3))
struct NestObject: public OProtoBase
{
    PROTO_FIELDS(NEST_OBJECT_FIELDS);
};

#define ROOT_FIELDS  ((int32_t, a1))((NestObject, v1))((std::string, s1))
struct RecursiveSerialize: public OProtoBase
{
    PROTO_FIELDS(ROOT_FIELDS);
};
PROTO_IMPLEMENTATION(NestObject, NEST_OBJECT_FIELDS)
PROTO_IMPLEMENTATION(RecursiveSerialize, ROOT_FIELDS)

BOOST_AUTO_TEST_CASE(RecursiveSerialize_None)
{
    RecursiveSerialize obj;
    obj.a1 = 2;
    obj.v1.n1 = 8;
    obj.v1.n2 = "shgli";
    obj.v1.n3 = 9.2;
    obj.s1 = "ok, i'm shgli";
    OSerializer serializer;
    serializer.RegisteProtoCreator(1
        , [](){ return new RecursiveSerialize(); }
        , OSerializer::DUMP_CREATOR);

    auto messages = std::move(serializer.Serialize(1, obj));
    auto pMessage = messages.get();

    BOOST_TEST(1 ==  pMessage->GetSequenceId());
    BOOST_TEST(1 ==  pMessage->GetMessageId());
    BOOST_TEST(-1 == pMessage->GetMessageSequenceId()); //only one message
    BOOST_TEST(0 == pMessage->GetPadNum());
    BOOST_TEST(0 == pMessage->GetCompressLevel());

    auto nestLen = sizeof(obj.v1.n1) + sizeof(int32_t) + obj.v1.n2.length() + sizeof(obj.v1.n3) ;
    auto rootLen = sizeof(obj.a1) + sizeof(int32_t) + obj.s1.length();
    BOOST_TEST((nestLen + rootLen) == pMessage->GetBodyLength());
    BOOST_TEST(false == pMessage->IsCompressed());
    BOOST_TEST(false == pMessage->IsEncrypted());

    RecursiveSerialize* deObj = serializer.CreateProto<RecursiveSerialize>(pMessage->GetMessageId());
    BOOST_TEST(true == serializer.Deserialize(*deObj, messages, OSerializer::NONE_KEY));
    BOOST_TEST(deObj->a1 == obj.a1);
    BOOST_TEST(deObj->v1.n1 == obj.v1.n1);
    BOOST_TEST(deObj->v1.n2 == obj.v1.n2);
    BOOST_TEST(deObj->v1.n3 == obj.v1.n3);
    BOOST_TEST(deObj->s1 == obj.s1);
}

BOOST_AUTO_TEST_CASE(RecursiveSerialize_Compress)
{
    RecursiveSerialize obj;
    obj.a1 = 2;
    obj.a1 = 2;
    obj.v1.n1 = 8;
    obj.v1.n2 = "shgli";
    obj.v1.n3 = 9.2;
    obj.s1 = "ok, i'm shgli";
    OSerializer serializer;
    serializer.RegisteProtoCreator(1
        , [](){ return new RecursiveSerialize(); }
        , OSerializer::DUMP_CREATOR);

    int32_t compressLevel = 2;
    auto messages = std::move(serializer.Serialize(1, obj, compressLevel));
    auto pMessage = messages.get();

    BOOST_TEST(1 ==  pMessage->GetSequenceId());
    BOOST_TEST(1 ==  pMessage->GetMessageId());
    BOOST_TEST(-1 == pMessage->GetMessageSequenceId()); //only one message
    BOOST_TEST(0 == pMessage->GetPadNum());
    BOOST_TEST(2 == pMessage->GetCompressLevel());

    BOOST_TEST(true == pMessage->IsCompressed());
    BOOST_TEST(false == pMessage->IsEncrypted());

    RecursiveSerialize* deObj = serializer.CreateProto<RecursiveSerialize>(pMessage->GetMessageId());
    BOOST_TEST(true == serializer.Deserialize(*deObj, messages, OSerializer::NONE_KEY));
    BOOST_TEST(deObj->a1 == obj.a1);
    BOOST_TEST(deObj->v1.n1 == obj.v1.n1);
    BOOST_TEST(deObj->v1.n2 == obj.v1.n2);
    BOOST_TEST(deObj->v1.n3 == obj.v1.n3);
    BOOST_TEST(deObj->s1 == obj.s1);
}

BOOST_AUTO_TEST_CASE(RecursiveSerialize_Encrypt)
{
    RecursiveSerialize obj;
    obj.a1 = 2;
    obj.a1 = 2;
    obj.v1.n1 = 8;
    obj.v1.n2 = "shgli";
    obj.v1.n3 = 9.2;
    obj.s1 = "ok, i'm shgli";
    OSerializer serializer;
    serializer.RegisteProtoCreator(1
        , [](){ return new RecursiveSerialize(); }
        , OSerializer::DUMP_CREATOR);

    std::string key("test key");
    auto messages = std::move(serializer.Serialize(1, obj, key));
    auto pMessage = messages.get();

    BOOST_TEST(1 ==  pMessage->GetSequenceId());
    BOOST_TEST(1 ==  pMessage->GetMessageId());
    BOOST_TEST(-1 == pMessage->GetMessageSequenceId()); //only one message
    BOOST_TEST(0 == pMessage->GetCompressLevel());

    BOOST_TEST(false == pMessage->IsCompressed());
    BOOST_TEST(true == pMessage->IsEncrypted());

    RecursiveSerialize* deObj = serializer.CreateProto<RecursiveSerialize>(pMessage->GetMessageId());
    BOOST_TEST(true == serializer.Deserialize(*deObj, messages, key));
    BOOST_TEST(deObj->a1 == obj.a1);
    BOOST_TEST(deObj->v1.n1 == obj.v1.n1);
    BOOST_TEST(deObj->v1.n2 == obj.v1.n2);
    BOOST_TEST(deObj->v1.n3 == obj.v1.n3);
    BOOST_TEST(deObj->s1 == obj.s1);

    std::string errKey("err key");
    RecursiveSerialize* deObj1 = serializer.CreateProto<RecursiveSerialize>(pMessage->GetMessageId());
    BOOST_TEST(false == serializer.Deserialize(*deObj, messages, errKey));
    BOOST_TEST(deObj1->a1 != obj.a1);
    BOOST_TEST(deObj1->v1.n1 != obj.v1.n1);
    BOOST_TEST(deObj1->v1.n2 != obj.v1.n2);
    BOOST_TEST(deObj1->v1.n3 != obj.v1.n3);
    BOOST_TEST(deObj1->s1 != obj.s1);
}

BOOST_AUTO_TEST_CASE(RecursiveSerialize_Compress_And_Encrypt)
{
    RecursiveSerialize obj;
    obj.a1 = 2;
    obj.a1 = 2;
    obj.v1.n1 = 8;
    obj.v1.n2 = "shgli";
    obj.v1.n3 = 9.2;
    obj.s1 = "ok, i'm shgli";
    OSerializer serializer;
    serializer.RegisteProtoCreator(1
        , [](){ return new RecursiveSerialize(); }
        , OSerializer::DUMP_CREATOR);

    int32_t compressLevel = 2;
    std::string key("test key");
    auto messages = std::move(serializer.Serialize(1, obj, compressLevel, key));
    auto pMessage = messages.get();

    BOOST_TEST(1 ==  pMessage->GetSequenceId());
    BOOST_TEST(1 ==  pMessage->GetMessageId());
    BOOST_TEST(-1 == pMessage->GetMessageSequenceId()); //only one message
    BOOST_TEST(2 == pMessage->GetCompressLevel());

    BOOST_TEST(true == pMessage->IsCompressed());
    BOOST_TEST(true == pMessage->IsEncrypted());

    RecursiveSerialize* deObj = serializer.CreateProto<RecursiveSerialize>(pMessage->GetMessageId());
    BOOST_TEST(true == serializer.Deserialize(*deObj, messages, key));
    BOOST_TEST(deObj->a1 == obj.a1);
    BOOST_TEST(deObj->v1.n1 == obj.v1.n1);
    BOOST_TEST(deObj->v1.n2 == obj.v1.n2);
    BOOST_TEST(deObj->v1.n3 == obj.v1.n3);
    BOOST_TEST(deObj->s1 == obj.s1);

    std::string errKey("err key");
    RecursiveSerialize* deObj1 = serializer.CreateProto<RecursiveSerialize>(pMessage->GetMessageId());
    BOOST_TEST(false == serializer.Deserialize(*deObj, messages, errKey));
    BOOST_TEST(deObj1->a1 != obj.a1);
    BOOST_TEST(deObj1->v1.n1 != obj.v1.n1);
    BOOST_TEST(deObj1->v1.n2 != obj.v1.n2);
    BOOST_TEST(deObj1->v1.n3 != obj.v1.n3);
    BOOST_TEST(deObj1->s1 != obj.s1);
}
