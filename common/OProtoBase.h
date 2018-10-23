#ifndef _OLDPART_OPROTO_BASE_H
#define _OLDPART_OPROTO_BASE_H
#include <tuple>
#include <stdint.h>
#include <memory>
#include <type_traits>
#include <boost/call_traits.hpp>
#include <boost/coroutine2/all.hpp>
#include <boost/preprocessor/seq/for_each_i.hpp>

class OProtoBase;
struct OProtoSerializeHelperBase
{
    static int32_t MAX_MESSAGE_BODY_LENGTH;
    struct Buf
    {
        uint8_t* buf{nullptr};
        int32_t bufLen{0};
    };

    struct BufT
    {
        std::shared_ptr<Buf> buf;
        int32_t padNum;
        int32_t checksum;
        int32_t isLast;
    };

    using Coro = boost::coroutines2::coroutine<BufT>;
    static std::shared_ptr<Buf> EnsureBuffer(Coro::push_type& yield
        , std::shared_ptr<Buf> buf
        , int32_t& offset
        , int32_t eleSize);

    static std::shared_ptr<Buf> MakeBuffer(int32_t bufLen);
};

template<typename T, typename E> struct OProtoSerializeHelper;

template<typename T>
struct OProtoSerializeHelper<T, std::enable_if_t<std::is_base_of<OProtoBase,T>::value>> : public OProtoSerializeHelperBase
{
    static int32_t Write(Coro::push_type& yield
        , std::shared_ptr<Buf>& buf
        , int32_t offset
        , typename boost::call_traits<T>::param_type v)
    {
        return v.Write(yield, buf, offset);
    }

    static int32_t Read(Coro::pull_type& pull
        , std::shared_ptr<Buf>& buf
        , int32_t offset
        , T& v)
    {
        return v.Read(pull, buf, offset);
    }
};

template<typename T>
struct OProtoSerializeHelper<T, std::enable_if_t<std::is_pod<T>::value>> : public OProtoSerializeHelperBase
{

    static int32_t Write(Coro::push_type& yield
        , std::shared_ptr<Buf>& buf
        , int32_t offset
        , typename boost::call_traits<T>::param_type v)
    {
        buf = EnsureBuffer(yield, buf, offset, sizeof(T));
	    std::copy_n((uint8_t*)&v, sizeof(v), buf->buf+offset);
        return offset+sizeof(v);
    }

    static int32_t Read(Coro::pull_type& pull
        , std::shared_ptr<Buf>& buf
        , int32_t offset
        , T& v)
    {
        static const int32_t FIELD_SIZE = sizeof(T);
        int32_t readableLen = buf->bufLen - offset;
        if(FIELD_SIZE <= readableLen)
        {
	        std::copy_n(buf->buf + offset, FIELD_SIZE, (uint8_t*)&v);
            return offset + FIELD_SIZE;
        }
        else
        {
            std::copy_n(buf->buf + offset, readableLen, (uint8_t*)&v);
            pull();
            auto x = pull.get();
            buf = x.buf;
            int8_t* pV = reinterpret_cast<int8_t*>(&v)+readableLen;
            int32_t restLen = FIELD_SIZE-readableLen;
            std::copy_n(buf->buf, restLen, pV);

            return FIELD_SIZE - readableLen;
        }
    }

};

template<>
struct OProtoSerializeHelper<std::string, void>: public OProtoSerializeHelperBase

{
    static int32_t Write(Coro::push_type& yield
        , std::shared_ptr<Buf>& buf
        , int32_t offset
        , const std::string& v);

    static int32_t Read(Coro::pull_type& yield
        , std::shared_ptr<Buf>& buf
        , int32_t offset
        , std::string& v);
};

template<typename T>
struct OProtoSerializeHelper<std::vector<T>, void> : public OProtoSerializeHelperBase
{
    static int32_t Write(Coro::push_type& yield
        , std::shared_ptr<Buf>& buf
        , int32_t offset
        , const std::vector<T>& v)
    {
        buf = EnsureBuffer(yield, buf, offset, sizeof(T));
        offset = OProtoSerializeHelper<int32_t,void>::Write(yield, buf, offset, v.size());
        for(auto& item : v)
        {
            offset = OProtoSerializeHelper<T,void>::Write(yield, buf, offset, item);
        }
        return offset;
    }

    static int32_t Read(Coro::pull_type& pull
        , std::shared_ptr<Buf>& buf
        , int32_t offset
        , std::vector<T>& v)
    {
        int32_t size = 0;
        offset = OProtoSerializeHelper<int32_t,void>::Read(pull, buf, offset, size);
        assert(size >= 0);
        v.resize(size);
        for(int32_t item = 0; item < size; ++item)
        {
            offset = OProtoSerializeHelper<T,void>::Read(pull, buf, offset, v[item]);
        }

        return offset;
    }
};

class OProtoBase
{
public:
    using Buf = OProtoSerializeHelperBase::Buf;
    using BufT = OProtoSerializeHelperBase::BufT;
    using Coro = OProtoSerializeHelperBase::Coro;
    virtual int32_t Write(Coro::push_type& yield, std::shared_ptr<Buf>& buf, int32_t offset) const = 0;
    virtual int32_t Read(Coro::pull_type& source, std::shared_ptr<Buf>& buf, int32_t offset) = 0;

    template<typename T>
    static int32_t WriteField(Coro::push_type& yield
        , std::shared_ptr<Buf>& buf
        , int32_t offset
        , typename boost::call_traits<T>::param_type v)
    {
	    return OProtoSerializeHelper<T,void>::Write(yield, buf, offset, v);
    }

    template<typename T>
    static int32_t ReadField(Coro::pull_type& pull
        , std::shared_ptr<Buf>& buf
        , int32_t offset
        , T& v)
    {
	    return OProtoSerializeHelper<T,void>::Read(pull, buf, offset, v);
    }
};

#define DECLARE_PROTO(pname) struct pname: public QProtoBase
#define PROTO_EXPAND_FIELD(z,d,i,e) BOOST_PP_TUPLE_ELEM(2,0,e) BOOST_PP_TUPLE_ELEM(2,1,e);
#define PROTO_WRITE_FIELD(z,d,i,e) offset = WriteField<BOOST_PP_TUPLE_ELEM(2,0,e)>(yield, buf, offset, BOOST_PP_TUPLE_ELEM(2,1,e));
#define PROTO_READ_FIELD(z,d,i,e) offset = ReadField<BOOST_PP_TUPLE_ELEM(2,0,e)>(pull, buf, offset, BOOST_PP_TUPLE_ELEM(2,1,e));


#define PROTO_FIELDS(typeNamePair) BOOST_PP_SEQ_FOR_EACH_I(PROTO_EXPAND_FIELD, ~, typeNamePair)\
    int32_t Write(Coro::push_type& yield, std::shared_ptr<Buf>& buf, int32_t offset) const override;\
    int32_t Read(Coro::pull_type& source, std::shared_ptr<Buf>& buf, int32_t offset) override;

#define PROTO_IMPLEMENTATION(cls, typeNamePair)\
    int32_t cls::Write(Coro::push_type& yield, std::shared_ptr<Buf>& buf, int32_t offset) const\
    {\
        BOOST_PP_SEQ_FOR_EACH_I(PROTO_WRITE_FIELD, ~, typeNamePair)\
        return offset;\
    }\
    int32_t cls::Read(Coro::pull_type& pull, std::shared_ptr<Buf>& buf, int32_t offset)\
    {\
        if(nullptr == buf)\
        { \
            auto x = pull.get();\
            buf = x.buf;\
            offset = 0; \
        }\
        BOOST_PP_SEQ_FOR_EACH_I(PROTO_READ_FIELD, ~, typeNamePair);\
        return offset;\
    }

#endif /* end of include guard: _OLDPART_OPROTO_BASE_H */
