
#pragma once
#include <stratosphere.hpp>

namespace flora
{
    struct GenericManagerOptions
    {
        static const size_t PointerBufferSize = 0x500;
        static const size_t MaxDomains = 4;
        static const size_t MaxDomainObjects = 0x100;
    };

    using ServerManager = WaitableManager<GenericManagerOptions>;
}