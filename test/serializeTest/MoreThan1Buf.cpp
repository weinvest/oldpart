#include <cstdint>
#include <boost/test/included/unit_test.hpp>
#include "common/OProtoBase.h"
#include "common/OMessage.h"
#include "common/OSerializer.h"
#define LESS_THAN_1BUF_FIELDS  ((int32_t, a1))((double, v1))((std::string, s1))
struct MoreThan1Buf: public OProtoBase
{
    PROTO_FIELDS(LESS_THAN_1BUF_FIELDS);
};

struct MaxBodyLenFixture
{
    int32_t mOldMaxLen;
    MaxBodyLenFixture()
        :mOldMaxLen(OProtoSerializeHelperBase::MAX_MESSAGE_BODY_LENGTH)
    {
    }

    ~MaxBodyLenFixture()
    {
        OProtoSerializeHelperBase::MAX_MESSAGE_BODY_LENGTH = mOldMaxLen;
    }
};
PROTO_IMPLEMENTATION(MoreThan1Buf, LESS_THAN_1BUF_FIELDS)

BOOST_FIXTURE_TEST_CASE(MoreThan1Buf_None, MaxBodyLenFixture)
{
    MoreThan1Buf obj;
    obj.a1 = 2;
    obj.v1 = 3.2;
    obj.s1 = "ok, i'm big big big message, hahahaha";
    OProtoSerializeHelperBase::MAX_MESSAGE_BODY_LENGTH = 32;

    OSerializer serializer;
    serializer.RegisteProtoCreator(1
        , [](){ return new MoreThan1Buf(); }
        , OSerializer::DUMP_CREATOR);

    auto messages = std::move(serializer.Serialize(1, obj));
    auto pMessage1 = std::move(messages.get());
    messages();
    auto pMessage2 = std::move(messages.get());
    messages();
    messages.get();
    BOOST_TEST(1 ==  pMessage1->GetSequenceId());
    BOOST_TEST(2 ==  pMessage2->GetSequenceId());

    BOOST_TEST(1 ==  pMessage1->GetMessageId());
    BOOST_TEST(1 ==  pMessage2->GetMessageId());

    BOOST_TEST(1 == pMessage1->GetMessageSequenceId());
    BOOST_TEST(-2 == pMessage2->GetMessageSequenceId());
    BOOST_TEST(false == pMessage1->IsLast());
    BOOST_TEST(true == pMessage2->IsLast());

    BOOST_TEST(0 == pMessage1->GetPadNum());
    BOOST_TEST(0 == pMessage2->GetPadNum());

    BOOST_TEST(0 == pMessage1->GetCompressLevel());
    BOOST_TEST(0 == pMessage2->GetCompressLevel());

    BOOST_TEST(OProtoSerializeHelperBase::MAX_MESSAGE_BODY_LENGTH == pMessage1->GetBodyLength());
    BOOST_TEST(false == pMessage1->IsCompressed());
    BOOST_TEST(false == pMessage2->IsCompressed());
    BOOST_TEST(false == pMessage1->IsEncrypted());
    BOOST_TEST(false == pMessage2->IsEncrypted());

    MoreThan1Buf* deObj = serializer.CreateProto<MoreThan1Buf>(pMessage1->GetMessageId());

    decltype(messages) used4Deserialize([pMessage1, pMessage2](auto& sink)
    {
        sink(pMessage1);
        sink(pMessage2);
    });
    BOOST_TEST(true == serializer.Deserialize(*deObj, used4Deserialize, OSerializer::NONE_KEY));


    BOOST_TEST(deObj->a1 == obj.a1);
    BOOST_TEST(deObj->v1 == obj.v1);
    BOOST_TEST(deObj->s1 == obj.s1);
}

BOOST_FIXTURE_TEST_CASE(MoreThan1Buf_Compress, MaxBodyLenFixture)
{
    MoreThan1Buf obj;
    obj.a1 = 2;
    obj.v1 = 3.2;
    obj.s1 = "ok, i'm big big big message, hahahaha";
    OProtoSerializeHelperBase::MAX_MESSAGE_BODY_LENGTH = 32;

    OSerializer serializer;
    serializer.RegisteProtoCreator(1
        , [](){ return new MoreThan1Buf(); }
        , OSerializer::DUMP_CREATOR);

    auto compressLevel = 2;
    auto messages = std::move(serializer.Serialize(1, obj, compressLevel));
    auto pMessage1 = std::move(messages.get());
    messages();
    auto pMessage2 = std::move(messages.get());
    messages();
    messages.get();
    BOOST_TEST(1 ==  pMessage1->GetSequenceId());
    BOOST_TEST(2 ==  pMessage2->GetSequenceId());

    BOOST_TEST(1 ==  pMessage1->GetMessageId());
    BOOST_TEST(1 ==  pMessage2->GetMessageId());

    BOOST_TEST(1 == pMessage1->GetMessageSequenceId());
    BOOST_TEST(-2 == pMessage2->GetMessageSequenceId());
    BOOST_TEST(false == pMessage1->IsLast());
    BOOST_TEST(true == pMessage2->IsLast());

    BOOST_TEST(0 == pMessage1->GetPadNum());
    BOOST_TEST(0 == pMessage2->GetPadNum());

    BOOST_TEST(compressLevel == pMessage1->GetCompressLevel());
    BOOST_TEST(compressLevel == pMessage2->GetCompressLevel());

    BOOST_TEST(OProtoSerializeHelperBase::MAX_MESSAGE_BODY_LENGTH == pMessage1->GetBodyLength());
    BOOST_TEST(true == pMessage1->IsCompressed());
    BOOST_TEST(true == pMessage2->IsCompressed());
    BOOST_TEST(false == pMessage1->IsEncrypted());
    BOOST_TEST(false == pMessage2->IsEncrypted());

    MoreThan1Buf* deObj = serializer.CreateProto<MoreThan1Buf>(pMessage1->GetMessageId());

    decltype(messages) used4Deserialize([pMessage1, pMessage2](auto& sink)
    {
        sink(pMessage1);
        sink(pMessage2);
    });
    BOOST_TEST(true == serializer.Deserialize(*deObj, used4Deserialize, OSerializer::NONE_KEY));


    BOOST_TEST(deObj->a1 == obj.a1);
    BOOST_TEST(deObj->v1 == obj.v1);
    BOOST_TEST(deObj->s1 == obj.s1);
}

BOOST_FIXTURE_TEST_CASE(MoreThan1Buf_Encrypt, MaxBodyLenFixture)
{
    MoreThan1Buf obj;
    obj.a1 = 2;
    obj.v1 = 3.2;
    obj.s1 = "ok, i'm big big big message, hahahaha";
    OProtoSerializeHelperBase::MAX_MESSAGE_BODY_LENGTH = 32;

    OSerializer serializer;
    serializer.RegisteProtoCreator(1
        , [](){ return new MoreThan1Buf(); }
        , OSerializer::DUMP_CREATOR);

    std::string key("test key");
    auto messages = std::move(serializer.Serialize(1, obj, key));
    auto pMessage1 = std::move(messages.get());
    messages();
    auto pMessage2 = std::move(messages.get());
    messages();
    auto pMessage3 = messages.get();

    BOOST_TEST(1 ==  pMessage1->GetSequenceId());
    BOOST_TEST(2 ==  pMessage2->GetSequenceId());

    BOOST_TEST(1 ==  pMessage1->GetMessageId());
    BOOST_TEST(1 ==  pMessage2->GetMessageId());

    BOOST_TEST(1 == pMessage1->GetMessageSequenceId());
    BOOST_TEST(-2 == pMessage2->GetMessageSequenceId());
    BOOST_TEST(false == pMessage1->IsLast());
    BOOST_TEST(true == pMessage2->IsLast());

    BOOST_TEST(0 == pMessage1->GetCompressLevel());
    BOOST_TEST(0 == pMessage2->GetCompressLevel());

    BOOST_TEST(OProtoSerializeHelperBase::MAX_MESSAGE_BODY_LENGTH == pMessage1->GetBodyLength());
    BOOST_TEST(false == pMessage1->IsCompressed());
    BOOST_TEST(false == pMessage2->IsCompressed());
    BOOST_TEST(true == pMessage1->IsEncrypted());
    BOOST_TEST(true == pMessage2->IsEncrypted());

    MoreThan1Buf* deObj = serializer.CreateProto<MoreThan1Buf>(pMessage1->GetMessageId());

    decltype(messages) used4Deserialize([pMessage1, pMessage2](auto& sink)
    {
        sink(pMessage1);
        sink(pMessage2);
    });
    BOOST_TEST(true == serializer.Deserialize(*deObj, used4Deserialize, key));

    BOOST_TEST(deObj->a1 == obj.a1);
    BOOST_TEST(deObj->v1 == obj.v1);
    BOOST_TEST(deObj->s1 == obj.s1);
}

BOOST_AUTO_TEST_CASE(MoreThan1Buf_Compress_And_Encrypt)
{

}
