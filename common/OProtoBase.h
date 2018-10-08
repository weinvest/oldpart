#ifndef _OLDPART_OPROTO_BASE_H
#define _OLDPART_OPROTO_BASE_H
#include <boost/coroutines2/all.hpp>

class OProtoBase
{
public:
    using Coro = oost::coroutines2::coroutine<uint8_t*>;
    virtual int32_t Write(Coro::push_type& yield, uint8_t* buf, int32_t offset) = 0;
    virtual int32_t Read(Coro::pull_type& source, uint8_t* buf, int32_t offset) = 0;
};
#endif /* end of include guard: _OLDPART_OPROTO_BASE_H */
