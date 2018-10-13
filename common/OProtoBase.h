#ifndef _OLDPART_OPROTO_BASE_H
#define _OLDPART_OPROTO_BASE_H
#include <tuple>
#include <boost/coroutines2/all.hpp>

class OProtoBase
{
public:
    using Coro = oost::coroutines2::coroutine<std::pair<int32_t, uint8_t*>>;
    virtual void Write(Coro::push_type& yield, uint8_t* buf, int32_t offset) = 0;
    virtual void Read(Coro::pull_type& source, uint8_t* buf, int32_t offset) = 0;
};
#endif /* end of include guard: _OLDPART_OPROTO_BASE_H */
