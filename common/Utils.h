#ifndef _OLDPART_UTILS_H
#define _OLDPART_UTILS_H
#include <memory>

template <typename T>
std::shared_ptr<T> make_shared_array(size_t size)
{
    return std::shared_ptr<T>(new T[size], std::default_delete<T[]>());
}


#endif //_OLDPART_UTILS_H

