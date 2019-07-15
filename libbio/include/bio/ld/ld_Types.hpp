
#pragma once
#include <bio/bio_Types.hpp>

namespace bio::ld
{
    struct Module
    {
        u32 magic;
        u32 dynamic;
        u32 bss_start;
        u32 bss_end;
        u32 unwind_start;
        u32 unwind_end;
        u32 module_object;
    };
}