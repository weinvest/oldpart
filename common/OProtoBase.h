#ifndef _OLDPART_OPROTO_BASE_H
#define _OLDPART_OPROTO_BASE_H
#include <tuple>
#include <stdint.h>
#include <memory>
#include <boost/call_traits.hpp>
#include <boost/coroutine2/all.hpp>
#include <boost/preprocessor/seq/for_each_i.hpp>

struct OProtoSerializeHelperBase
{
    static constexpr int32_t MAX_MESSAGE_BODY_LENGTH = 1<<21;
    using BufT = std::tuple<std::shared_ptr<uint8_t>, int32_t, int32_t, int32_t>;
    using Coro = boost::coroutines2::coroutine<BufT>;
    static std::shared_ptr<uint8_t> EnsureBuffer(Coro::push_type& yield
        , std::shared_ptr<uint8_t>& buf
        , int32_t& offset
        , int32_t eleSize);
};

template<typename T>
struct OProtoSerializeHelper: public OProtoSerializeHelperBase
{
    static int32_t Write(Coro::push_type& yield
        , std::shared_ptr<uint8_t>& buf
        , int32_t offset
        , typename boost::call_traits<T>::param_type v)
    {
        buf = EnsureBuffer(yield, buf, offset, sizeof(T));
	    std::copy_n((uint8_t*)&v, sizeof(v), buf.get()+offset);
        return offset+sizeof(v);
    }

    static int32_t Read(Coro::pull_type& pull
        , std::shared_ptr<uint8_t>& buf
        , int32_t offset
        , T& v)
    {
        static const int32_t FIELD_SIZE = sizeof(T);
        int32_t readableLen = MAX_MESSAGE_BODY_LENGTH - offset;
        if(FIELD_SIZE <= readableLen)
        {
	        std::copy_n(buf.get() + offset, FIELD_SIZE, (uint8_t*)&v);
            return offset + FIELD_SIZE;
        }
        else
        {
            std::copy_n(buf.get() + offset, readableLen, (uint8_t*)&v);
            auto x = pull.get();
            buf = std::get<0>(x);
            int8_t* pV = reinterpret_cast<int8_t*>(&v)+readableLen;
            int32_t restLen = FIELD_SIZE-readableLen;
            std::copy_n(buf.get(), restLen, pV);

            return FIELD_SIZE - readableLen;
        }
    }

};

template<>
struct OProtoSerializeHelper<std::string>: public OProtoSerializeHelperBase

{
    static int32_t Write(Coro::push_type& yield
        , std::shared_ptr<uint8_t>& buf
        , int32_t offset
        , const std::string& v);

    static int32_t Read(Coro::pull_type& yield
        , std::shared_ptr<uint8_t>& buf
        , int32_t offset
        , std::string& v);
};

class OProtoBase
{
public:

    using BufT = OProtoSerializeHelperBase::BufT;
    using Coro = OProtoSerializeHelperBase::Coro;
    virtual void Write(Coro::push_type& yield, std::shared_ptr<uint8_t> buf, int32_t offset) const = 0;
    virtual void Read(Coro::pull_type& source, std::shared_ptr<uint8_t> buf, int32_t offset) = 0;

    template<typename T>
    static int32_t WriteField(Coro::push_type& yield
        , std::shared_ptr<uint8_t>& buf
        , int32_t offset
        , typename boost::call_traits<T>::param_type v)
    {
	return OProtoSerializeHelper<T>::Write(yield, buf, offset, v);
    }

    template<typename T>
    static int32_t ReadField(Coro::pull_type& pull
        , std::shared_ptr<uint8_t>& buf
        , int32_t offset
        , T& v)
    {
	return OProtoSerializeHelper<T>::Read(pull, buf, offset, v);
    }
};

#define DECLARE_PROTO(pname) struct pname: public QProtoBase
#define PROTO_EXPAND_FIELD(z,d,i,e) BOOST_PP_TUPLE_ELEM(2,0,e) BOOST_PP_TUPLE_ELEM(2,1,e);
#define PROTO_WRITE_FIELD(z,d,i,e) offset = WriteField<BOOST_PP_TUPLE_ELEM(2,0,e)>(yield, buf, offset, BOOST_PP_TUPLE_ELEM(2,1,e));
#define PROTO_READ_FIELD(z,d,i,e) offset = ReadField<BOOST_PP_TUPLE_ELEM(2,0,e)>(pull, buf, offset, BOOST_PP_TUPLE_ELEM(2,1,e));


#define PROTO_FIELDS(typeNamePair) BOOST_PP_SEQ_FOR_EACH_I(PROTO_EXPAND_FIELD, ~, typeNamePair)\
    void Write(Coro::push_type& yield, std::shared_ptr<uint8_t> buf, int32_t offset) const override;\
    void Read(Coro::pull_type& source, std::shared_ptr<uint8_t> buf, int32_t offset) override;

#define PROTO_IMPLEMENTATION(cls, typeNamePair)\
    void cls::Write(Coro::push_type& yield, std::shared_ptr<uint8_t> buf, int32_t offset) const\
    {\
        BOOST_PP_SEQ_FOR_EACH_I(PROTO_WRITE_FIELD, ~, typeNamePair)\
        yield(std::make_tuple(buf, offset, 0, 0));\
    }\
    void cls::Read(Coro::pull_type& pull, std::shared_ptr<uint8_t> buf, int32_t offset)\
    {\
        if(nullptr == buf){ buf = std::get<0>(pull.get()); offset = 0; }\
        BOOST_PP_SEQ_FOR_EACH_I(PROTO_READ_FIELD, ~, typeNamePair);\
    }

// #undef PROTO_EXPAND_FIELD
// #undef PROTO_WRITE_FIELD
// #undef PROTO_READ_FIELD

#endif /* end of include guard: _OLDPART_OPROTO_BASE_H */
