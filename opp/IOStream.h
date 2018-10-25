#ifndef _OLDPART_IOSTREAM_H
#define _OLDPART_IOSTREAM_H
#include <iostream>
#include "common/OProtoBase.h"
class IOStream: public OProtoBase
{
public:
    int32_t Write(Coro::push_type& yield, std::shared_ptr<Buf>& buf, int32_t offset) const override;
    int32_t Read(Coro::pull_type& source, std::shared_ptr<Buf>& buf, int32_t offset) override;

    int32_t size{0};
    std::iostream *stream{nullptr};
};
#endif //_OLDPART_OSTREAM_H
