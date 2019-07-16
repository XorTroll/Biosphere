
#pragma once
#include <bio/bio_Types.hpp>

namespace bio::env
{
    struct ABIConfigEntry
    {
        u32 key;
        u32 flags;
        u64 value[2];
    };

    enum class ABIConfigKey
    {
        EOL = 0,
        MainThreadHandle = 1,
        NextLoadPath = 2,
        OverrideHeap = 3,
        OverrideService = 4,
        Argv = 5,
        SyscallAvailableHint = 6,
        AppletType = 7,
        AppletWorkaround = 8,
        Reserved9 = 9,
        ProcessHandle = 10,
        LastLoadResult = 11,
        RandomSeed = 14,
    };

    enum class ABIEntryFlags
    {
        Mandatory = (1 << 0)
    };

    inline ABIEntryFlags operator|(ABIEntryFlags l, ABIEntryFlags r)
    {
        return static_cast<ABIEntryFlags>(static_cast<u32>(l) | static_cast<u32>(r));
    }

    enum class ABIAppletFlags
    {
        ApplicationOverride = (1 << 0)
    };

    inline ABIAppletFlags operator|(ABIAppletFlags l, ABIAppletFlags r)
    {
        return static_cast<ABIAppletFlags>(static_cast<u32>(l) | static_cast<u32>(r));
    }

    typedef void BIO_NORETURN (*TerminateFunction)(int res);
}