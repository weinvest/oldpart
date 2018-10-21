#include <cstdint>
#define BOOST_TEST_DYN_LINK
#include <vector>
#include <boost/test/unit_test.hpp>
#include "common/OProtoBase.h"
#include "common/OMessage.h"
#include "common/OSerializer.h"
#define NEST_OBJECT_FIELDS ((uint8_t, n1))((std::string, n2))((double, n3))
struct NestObject1: public OProtoBase
{
    PROTO_FIELDS(NEST_OBJECT_FIELDS);
};

#define ROOT_FIELDS  ((int32_t, a1))((std::vector<NestObject1>, v1))((std::string, s1))
struct VectorSerialize: public OProtoBase
{
    PROTO_FIELDS(ROOT_FIELDS);
};
PROTO_IMPLEMENTATION(NestObject1, NEST_OBJECT_FIELDS)
PROTO_IMPLEMENTATION(VectorSerialize, ROOT_FIELDS)

BOOST_AUTO_TEST_CASE(VectorSerialize_None)
{
    VectorSerialize obj;
    obj.a1 = 2;
    obj.v1.resize(2);
    obj.v1[0].n1 = 0;
    obj.v1[0].n2 = "item1";
    obj.v1[0].n3 = 3.2;

    obj.v1[1].n1 = 1;
    obj.v1[1].n2 = "item2";
    obj.v1[1].n3 = 4.2;

    obj.s1 = "ok, i'm shgli";

    OSerializer serializer;
    serializer.RegisteProtoCreator(1
        , [](){ return new VectorSerialize(); }
        , OSerializer::DUMP_CREATOR);

    auto messages = std::move(serializer.Serialize(1, obj));
    auto pMessage = messages.get();

    BOOST_TEST(1 ==  pMessage->GetSequenceId());
    BOOST_TEST(1 ==  pMessage->GetMessageId());
    BOOST_TEST(-1 == pMessage->GetMessageSequenceId()); //only one message
    BOOST_TEST(0 == pMessage->GetPadNum());
    BOOST_TEST(0 == pMessage->GetCompressLevel());

    auto item1Len = sizeof(obj.v1[0].n1) + sizeof(int32_t) + obj.v1[0].n2.length() + sizeof(obj.v1[0].n3);
    auto item2Len = sizeof(obj.v1[1].n1) + sizeof(int32_t) + obj.v1[1].n2.length() + sizeof(obj.v1[1].n3);
    auto rootLen = sizeof(obj.a1) + sizeof(int32_t) + obj.s1.length();
    auto totalLen = rootLen + sizeof(int32_t) + item1Len + item2Len;
    BOOST_TEST(totalLen == pMessage->GetBodyLength());
    BOOST_TEST(false == pMessage->IsCompressed());
    BOOST_TEST(false == pMessage->IsEncrypted());

    VectorSerialize* deObj = serializer.CreateProto<VectorSerialize>(pMessage->GetMessageId());
    BOOST_TEST(true == serializer.Deserialize(*deObj, messages, OSerializer::NONE_KEY));
    BOOST_TEST(deObj->a1 == obj.a1);
    BOOST_TEST(deObj->v1[0].n1 == obj.v1[0].n1);
    BOOST_TEST(deObj->v1[0].n2 == obj.v1[0].n2);
    BOOST_TEST(deObj->v1[0].n3 == obj.v1[0].n3);
    BOOST_TEST(deObj->v1[1].n1 == obj.v1[1].n1);
    BOOST_TEST(deObj->v1[1].n2 == obj.v1[1].n2);
    BOOST_TEST(deObj->v1[1].n3 == obj.v1[1].n3);
    BOOST_TEST(deObj->s1 == obj.s1);
}

BOOST_AUTO_TEST_CASE(VectorSerialize_Compress)
{
    VectorSerialize obj;
    obj.a1 = 2;
    obj.v1.resize(2);
    obj.v1[0].n1 = 0;
    obj.v1[0].n2 = "item1";
    obj.v1[0].n3 = 3.2;

    obj.v1[1].n1 = 1;
    obj.v1[1].n2 = "item2";
    obj.v1[1].n3 = 4.2;

    obj.s1 = "ok, i'm shgli";

    OSerializer serializer;
    serializer.RegisteProtoCreator(1
        , [](){ return new VectorSerialize(); }
        , OSerializer::DUMP_CREATOR);

    int32_t compressLevel = 2;
    auto messages = std::move(serializer.Serialize(1, obj, compressLevel));
    auto pMessage = messages.get();

    BOOST_TEST(1 ==  pMessage->GetSequenceId());
    BOOST_TEST(1 ==  pMessage->GetMessageId());
    BOOST_TEST(-1 == pMessage->GetMessageSequenceId()); //only one message
    BOOST_TEST(0 == pMessage->GetPadNum());
    BOOST_TEST(compressLevel == pMessage->GetCompressLevel());

    BOOST_TEST(true == pMessage->IsCompressed());
    BOOST_TEST(false == pMessage->IsEncrypted());

    VectorSerialize* deObj = serializer.CreateProto<VectorSerialize>(pMessage->GetMessageId());
    BOOST_TEST(true == serializer.Deserialize(*deObj, messages, OSerializer::NONE_KEY));
    BOOST_TEST(deObj->a1 == obj.a1);
    BOOST_TEST(deObj->v1[0].n1 == obj.v1[0].n1);
    BOOST_TEST(deObj->v1[0].n2 == obj.v1[0].n2);
    BOOST_TEST(deObj->v1[0].n3 == obj.v1[0].n3);
    BOOST_TEST(deObj->v1[1].n1 == obj.v1[1].n1);
    BOOST_TEST(deObj->v1[1].n2 == obj.v1[1].n2);
    BOOST_TEST(deObj->v1[1].n3 == obj.v1[1].n3);
    BOOST_TEST(deObj->s1 == obj.s1);
}

BOOST_AUTO_TEST_CASE(VectorSerialize_Encrypt)
{
    VectorSerialize obj;
    obj.a1 = 2;
    obj.v1.resize(2);
    obj.v1[0].n1 = 0;
    obj.v1[0].n2 = "item1";
    obj.v1[0].n3 = 3.2;

    obj.v1[1].n1 = 1;
    obj.v1[1].n2 = "item2";
    obj.v1[1].n3 = 4.2;

    obj.s1 = "ok, i'm shgli";

    OSerializer serializer;
    serializer.RegisteProtoCreator(1
        , [](){ return new VectorSerialize(); }
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

    VectorSerialize* deObj = serializer.CreateProto<VectorSerialize>(pMessage->GetMessageId());
    BOOST_TEST(true == serializer.Deserialize(*deObj, messages, key));
    BOOST_TEST(deObj->a1 == obj.a1);
    BOOST_TEST(deObj->v1[0].n1 == obj.v1[0].n1);
    BOOST_TEST(deObj->v1[0].n2 == obj.v1[0].n2);
    BOOST_TEST(deObj->v1[0].n3 == obj.v1[0].n3);
    BOOST_TEST(deObj->v1[1].n1 == obj.v1[1].n1);
    BOOST_TEST(deObj->v1[1].n2 == obj.v1[1].n2);
    BOOST_TEST(deObj->v1[1].n3 == obj.v1[1].n3);
    BOOST_TEST(deObj->s1 == obj.s1);

    std::string errKey("error key");
    VectorSerialize* errObj = serializer.CreateProto<VectorSerialize>(pMessage->GetMessageId());
    BOOST_TEST(false == serializer.Deserialize(*errObj, messages, errKey));
    BOOST_TEST(errObj->a1 != obj.a1);
    BOOST_TEST(errObj->v1.size() == 0);
    BOOST_TEST(errObj->s1 != obj.s1);
}

BOOST_AUTO_TEST_CASE(VectorSerialize_Compress_And_Encrypt)
{
    VectorSerialize obj;
    obj.a1 = 2;
    obj.v1.resize(2);
    obj.v1[0].n1 = 0;
    obj.v1[0].n2 = "item1";
    obj.v1[0].n3 = 3.2;

    obj.v1[1].n1 = 1;
    obj.v1[1].n2 = "item2";
    obj.v1[1].n3 = 4.2;

    obj.s1 = "ok, i'm shgli";

    OSerializer serializer;
    serializer.RegisteProtoCreator(1
        , [](){ return new VectorSerialize(); }
        , OSerializer::DUMP_CREATOR);

    std::string key("test key");
    int32_t compressLevel = 2;
    auto messages = std::move(serializer.Serialize(1, obj, compressLevel, key));
    auto pMessage = messages.get();

    BOOST_TEST(1 ==  pMessage->GetSequenceId());
    BOOST_TEST(1 ==  pMessage->GetMessageId());
    BOOST_TEST(-1 == pMessage->GetMessageSequenceId()); //only one message
    BOOST_TEST(compressLevel == pMessage->GetCompressLevel());

    BOOST_TEST(true == pMessage->IsCompressed());
    BOOST_TEST(true == pMessage->IsEncrypted());

    VectorSerialize* deObj = serializer.CreateProto<VectorSerialize>(pMessage->GetMessageId());
    BOOST_TEST(true == serializer.Deserialize(*deObj, messages, key));
    BOOST_TEST(deObj->a1 == obj.a1);
    BOOST_TEST(deObj->v1[0].n1 == obj.v1[0].n1);
    BOOST_TEST(deObj->v1[0].n2 == obj.v1[0].n2);
    BOOST_TEST(deObj->v1[0].n3 == obj.v1[0].n3);
    BOOST_TEST(deObj->v1[1].n1 == obj.v1[1].n1);
    BOOST_TEST(deObj->v1[1].n2 == obj.v1[1].n2);
    BOOST_TEST(deObj->v1[1].n3 == obj.v1[1].n3);
    BOOST_TEST(deObj->s1 == obj.s1);

    std::string errKey("error key");
    VectorSerialize* errObj = serializer.CreateProto<VectorSerialize>(pMessage->GetMessageId());
    BOOST_TEST(false == serializer.Deserialize(*errObj, messages, errKey));
    BOOST_TEST(errObj->a1 != obj.a1);
    BOOST_TEST(errObj->v1.size() == 0);
    BOOST_TEST(errObj->s1 != obj.s1);
}
