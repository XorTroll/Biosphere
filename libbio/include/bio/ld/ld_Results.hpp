
#pragma once
#include <bio/bio_Types.hpp>

namespace bio::ld
{
    static constexpr u32 SubmoduleBase = 200;

    BIO_DEFINERESULT(InvalidInputNro, 1)
    BIO_DEFINERESULT(MissingDtEntry, 2)
    BIO_DEFINERESULT(DuplicatedDtEntry, 3)
    BIO_DEFINERESULT(InvalidSymEnt, 4)
    BIO_DEFINERESULT(InvalidModuleState, 5)
    BIO_DEFINERESULT(InvalidRelocEnt, 6)
    BIO_DEFINERESULT(InvalidRelocTableSize, 7)
    BIO_DEFINERESULT(RelaUnsupportedSymbol, 8)
    BIO_DEFINERESULT(UnrecognizedRelocType, 9)
    BIO_DEFINERESULT(InvalidRelocTableType, 10)
    BIO_DEFINERESULT(NeedsSymTab, 11)
    BIO_DEFINERESULT(NeedsStrTab, 12)
    BIO_DEFINERESULT(CouldNotResolveSymbol, 13)
}