
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

    static constexpr u32 CurrentProcessPseudoHandle = 0xFFFF8001;

    Result CloseHandle(u32 handle);
    Result CreateEvent(Out<u32> w_end, Out<u32> r_end);
    Result ResetSignal(u32 signal);
    Result WaitSynchronization(Out<u32> handle_index, u32 *handles, u32 num_handles, u64 timeout);
    Result SignalEvent(u32 event);
    Result SendSyncRequest(u32 handle);
    Result ConnectToNamedPort(Out<u32> session_handle, const char *name);
    Result SleepThread(i64 nanoseconds);
    Result OutputDebugString(char *str, u64 size);
    Result SetHeapSize(Out<void*> address, u64 size);
    void BIO_NORETURN ExitProcess();
    void BIO_NORETURN ExitThread();
    Result GetInfo(u32 first_id, u32 second_id, u32 handle, Out<u64> info);
    Result QueryMemory(u64 address, Out<MemoryInfo> mem_info, Out<u32> page_info);
    Result CreateSharedMemory(Out<u32> handle, size_t size, u32 local_perms, u32 perms);
    Result MapSharedMemory(u32 handle, void *addr, size_t size, u32 perms);
    Result UnmapSharedMemory(u32 handle, void *addr, size_t size);
    Result CreateTransferMemory(Out<u32> handle, void *addr, u64 size, u32 permission);
    Result MapTransferMemory(u32 handle, void *addr, size_t size, u32 perms);
    Result UnmapTransferMemory(u32 handle, void *addr, size_t size);
}