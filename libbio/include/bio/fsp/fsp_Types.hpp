
#pragma once
#include <bio/bio_Types.hpp>

namespace bio::fsp
{
    enum class DirectoryEntryType
    {
        Directory,
        File,
    };

    enum class Partition
    {
        BootPartition1Root = 0,
        BootPartition2Root = 10,
        UserDataRoot = 20,
        BootConfigAndPackage2Part1 = 21,
        BootConfigAndPackage2Part2 = 22,
        BootConfigAndPackage2Part3 = 23,
        BootConfigAndPackage2Part4 = 24,
        BootConfigAndPackage2Part5 = 25,
        BootConfigAndPackage2Part6 = 26,
        CalibrationBinary = 27,
        CalibrationFile = 28,
        SafeMode = 29,
        SystemProperEncryption = 30,
        User = 31,
    };

    static constexpr size_t PathMax = 0x301;

    struct DirectoryEntry
    {
        const char path[PathMax];
        u32 unk;
        u32 entry_type;
        u64 file_size;
    };
}