
#pragma once
#include <bio/bio_Types.hpp>
#include <mutex>

namespace bio::os
{
    typedef void(*ResourceFinalizeFunction)();

    void AddFinalizeFunction(ResourceFinalizeFunction func);

    void CallFinalizeFunctions();
}