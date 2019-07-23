
#pragma once
#include <bio/bio_Types.hpp>

namespace bio::fs
{
    static constexpr u32 SubmoduleBase = 100;

    BIO_DEFINERESULT(InvalidOpenFlags, 1)
    BIO_DEFINERESULT(MountNameInUse, 2)
    BIO_DEFINERESULT(InvalidFile, 3)
    BIO_DEFINERESULT(EndOfDirectory, 4)
}