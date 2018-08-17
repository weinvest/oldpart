#ifndef _OLDPART_DISCOVER_H
#define _OLDPART_DISCOVER_H

using int32_t = int;
struct alignas(4) Discover
{
    int32_t magic; //>0: from ops; <0: from opc
    int32_t ip;
    int32_t port;
    char name[32];
};
#endif

