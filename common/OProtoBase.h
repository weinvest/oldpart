#ifndef _OLDPART_OPROTO_BASE_H
#define _OLDPART_OPROTO_BASE_H
#include <tuple>
#include <stdint.h>
#include <memory>
#include <boost/coroutine2/all.hpp>
#include <boost/preprocessor/seq/for_each_i.hpp>

class OProtoBase
{
public:
    using BufT = std::tuple<std::shared_ptr<uint8_t>, int32_t, int32_t>;
    using Coro = boost::coroutines2::coroutine<BufT>;
    virtual void Write(Coro::push_type& yield, std::shared_ptr<uint8_t> buf, int32_t offset) const = 0;
    virtual void Read(Coro::pull_type& source) = 0;
};

#define DECLARE_PROTO(pname) struct pname: public QProtoBase
#define PROTO_EXPAND_FIELD(z,d,i,e) BOOST_PP_TUPLE_ELEM(2,0,e) BOOST_PP_TUPLE_ELEM(2,1,e);
#define PROTO_WRITE_FIELD(z,d,i,e) offset = WritePOD(yield, buf, offset);
#define PROTO_FIELDS(typeNamePair) BOOST_PP_SEQ_FOR_EACH_I(PROTO_EXPAND_FIELD, ~, typeNamePair)\
    void Write(Coro::push_type& yield, std::shared_ptr<uint8_t> buf, int32_t offset) override\
    {\
        BOOST_PP_SEQ_FOR_EACH_I(PROTO_WRITE_FIELD, ~, typeNamePair)\
        yield(std::make_tuple(buf, offset, 0));\
    }\
    void Read(Coro::pull_type& source) override\
    {\
    }
#endif /* end of include guard: _OLDPART_OPROTO_BASE_H */
