#include <bio/os/os_TLS.hpp>

extern "C"
{
    extern void *__bio_os_GetTLS();
}

namespace bio::os
{
    void *GetTLS()
    {
        return __bio_os_GetTLS();
    }
}