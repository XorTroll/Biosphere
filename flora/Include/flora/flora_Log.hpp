
#pragma once
#include <switch.h>

namespace flora
{
    bool InitializeLogging();
    void FinalizeLogging();
    bool SendRaw(void *val, size_t size);
}