
#pragma once
#include <bio/bio_Types.hpp>

namespace bio::os
{
    struct Version
    {
        u8 major;
        u8 minor;
        u8 micro;
    };

    Version GetFirmwareVersion();
}