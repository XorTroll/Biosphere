
#pragma once
#include <bio/bio_Types.hpp>

namespace bio::svc
{
    struct MemoryInfo
    {
        u64 address;
        u64 size;
        u32 type;
        u32 attributes;
        u32 permissions;
        u32 device_ref_count;
        u32 ipc_ref_count;
        u32 pad;
    };

    struct BIO_PACKED SecureMonitorArgs
    {
        u64 args[8];
    };

    Result CloseHandle(u32 handle);
    Result CreateEvent(Out<u32> w_end, Out<u32> r_end);
    Result ResetSignal(u32 signal);
    Result WaitSynchronization(Out<u32> handle_index, u32 *handles, u32 num_handles, u64 timeout);
    Result SignalEvent(u32 event);
    Result CreateTransferMemory(Out<u32> handle, void *addr, u64 size, u32 permission);
    Result SendSyncRequest(u32 handle);
    Result ConnectToNamedPort(Out<u32> session_handle, const char *name);
    Result SleepThread(i64 nanoseconds);
    Result OutputDebugString(char *str, u64 size);
    Result SetHeapSize(Out<void*> address, u64 size);
}