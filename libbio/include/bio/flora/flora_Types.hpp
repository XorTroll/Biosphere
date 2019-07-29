
#pragma once
#include <bio/bio_Types.hpp>

namespace bio::flora
{
    #define BIO_FLORA_LOG_BASE(flora_session_ptr, mode, fmt, ...) do { \
        char loc[0x1000] = {0}; \
        sprintf(loc, "%s [%s:%d]", __FUNCTION__, __FILE__, __LINE__); \
        char log[0x1000] = {0}; \
        sprintf(log, fmt, __VA_ARGS__); \
        char fulllog[0x2000] = {0}; \
        sprintf(fulllog, "%s -> %s", loc, log); \
        flora_session_ptr->Write##mode(fulllog, strlen(fulllog)); \
        } while(0)

    #define BIO_FLORA_LOG(flora_session_ptr, fmt, ...) BIO_FLORA_LOG_BASE(flora_session_ptr, Out, fmt, ...)

    #define BIO_FLORA_ERR(flora_session_ptr, fmt, ...) BIO_FLORA_LOG_BASE(flora_session_ptr, Err, fmt, ...)
}