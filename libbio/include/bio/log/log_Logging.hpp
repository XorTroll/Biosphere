
#pragma once
#include <bio/bio_Types.hpp>
#include <cstdio>

namespace bio::log
{
    typedef void(*LogWriteFunction)(const void *ptr, size_t sz);

    void SetStdoutLoggingFunction(LogWriteFunction func);
    void SetStderrLoggingFunction(LogWriteFunction func);

    // Flora service/logging PC <-> console tool
    void FloraStdoutLoggingFunction(const void *ptr, size_t sz);
    void FloraStderrLoggingFunction(const void *ptr, size_t sz);

    // SVC -> OutputDebugString
    void SvcStdoutLoggingFunction(const void *ptr, size_t sz);
    void SvcStderrLoggingFunction(const void *ptr, size_t sz);

    #define BIO_DEBUG_LOG(fmt, ...) do { \
        char userfmt[0x1000] = {0}; \
        sprintf(userfmt, fmt, __VA_ARGS__); \
        char msg[0x2000] = {0}; \
        sprintf(msg, "[Debug log] %s", userfmt); \
        bio::log::SvcStdoutLoggingFunction(msg, 0x1010); \
        } while(0)
}