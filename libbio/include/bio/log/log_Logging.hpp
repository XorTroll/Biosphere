
#pragma once
#include <bio/bio_Types.hpp>
#include <cstdio>

#define BIO_LOG bio::log::LogFmt

namespace bio::log
{
    typedef void(*LogWriteFunction)(const void *ptr, size_t sz);

    void SetStdoutLoggingFunction(LogWriteFunction func);
    void SetStderrLoggingFunction(LogWriteFunction func);

    template<typename ...Args>
    void LogFmt(const char *fmt, Args &&...fmt_args)
    {
        setbuf(stdout, NULL);
        printf(fmt, fmt_args...);
    }

    // Flora service/logging PC <-> console tool
    void FloraStdoutLoggingFunction(const void *ptr, size_t sz);
    void FloraStderrLoggingFunction(const void *ptr, size_t sz);

    // SVC -> OutputDebugString
    void SvcStdoutLoggingFunction(const void *ptr, size_t sz);
    void SvcStderrLoggingFunction(const void *ptr, size_t sz);
}