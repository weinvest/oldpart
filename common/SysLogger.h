#pragma once

#include <syslog.h>
struct SysLogger
{
    SysLogger(const char *ident, int option, int facility)
    {
        openlog(ident, option, facility);
    }

    ~SysLogger()
    {
        closelog();
    }
};

