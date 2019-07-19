
#pragma once
#include <bio/bio_Types.hpp>
#include <cstdio>

// Simplifies things A lot
#define BIO_LOG bio::log::LogFmt

namespace bio::log
{
    typedef void(*LogWriteFunction)(const void *ptr, size_t sz);

    void SetLoggingFunction(LogWriteFunction func);

    template<typename ...Args>
    void LogFmt(const char *fmt, Args &&...fmt_args)
    {
        setbuf(stdout, NULL);
        printf(fmt, fmt_args...);
    }

    void SvcOutputLoggingFunction(const void *ptr, size_t sz);
}