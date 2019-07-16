#include <bio/svc/svc_Base.hpp>

extern "C"
{
    bio::u32 __bio_svc_SetHeapSize(void **OutAddress, bio::u64 Size);
    bio::u32 __bio_svc_SetMemoryPermission(void *StartAddress, bio::u64 Size, bio::u32 Permissions);
    bio::u32 __bio_svc_SetMemoryAttribute(void *StartAddress, bio::u64 Size, bio::u32 State0, bio::u32 State1);
    bio::u32 __bio_svc_MapMemory(void *DestAddress, void *SourceAddress, bio::u64 Size);
    bio::u32 __bio_svc_UnmapMemory(void *DestAddress, void *SourceAddress, bio::u64 Size);
    bio::u32 __bio_svc_QueryMemory(bio::svc::MemoryInfo *MemInfo, bio::u32 *PageInfo, bio::u64 Address);
    void BIO_NORETURN __bio_svc_ExitProcess(void);
    bio::u32 __bio_svc_CreateThread(bio::u32 *Out_ThreadHandle, void *EntryPoint, void *Arguments, void *StackTop, int Priority, int CPUID);
    bio::u32 __bio_svc_StartThread(bio::u32 ThreadHandle);
    void BIO_NORETURN __bio_svc_ExitThread(void);
    bio::u32 __bio_svc_SleepThread(bio::i64 NanoSeconds);
    bio::u32 __bio_svc_GetThreadPriority(bio::u32 *Out_Priority, bio::u32 ThreadHandle);
    bio::u32 __bio_svc_SetThreadPriority(bio::u32 ThreadHandle, bio::u32 Priority);
    bio::u32 __bio_svc_GetThreadCoreMask(int *Out_PreferredCore, bio::u32 *Out_AffinityMask, bio::u32 ThreadHandle);
    bio::u32 __bio_svc_SetThreadCoreMask(bio::u32 ThreadHandle, int PreferredCore, bio::u32 AffinityMask);
    bio::u32 __bio_svc_GetCurrentProcessorNumber(void);
    bio::u32 __bio_svc_SignalEvent(bio::u32 EventHandle);
    bio::u32 __bio_svc_ClearEvent(bio::u32 EventHandle);
    bio::u32 __bio_svc_MapSharedMemory(bio::u32 VarHandle, void *Address, size_t Size, bio::u32 Permissions);
    bio::u32 __bio_svc_UnmapSharedMemory(bio::u32 VarHandle, void *Address, size_t Size);
    bio::u32 __bio_svc_CreateTransferMemory(bio::u32 *Out_Handle, void *Address, size_t Size, bio::u32 Permissions);
    bio::u32 __bio_svc_CloseHandle(bio::u32 VarHandle);
    bio::u32 __bio_svc_ResetSignal(bio::u32 VarHandle);
    bio::u32 __bio_svc_WaitSynchronization(bio::u32 *Index, const bio::u32 *Handles, int HandleCount, bio::u64 Timeout);
    bio::u32 __bio_svc_CancelSynchronization(bio::u32 ThreadHandle);
    bio::u32 __bio_svc_ArbitrateLock(bio::u32 WaitTag, bio::u32 *TagLocation, bio::u32 SelfTag);
    bio::u32 __bio_svc_ArbitrateUnlock(bio::u32 *TagLocation);
    bio::u32 __bio_svc_WaitProcessWideKeyAtomic(bio::u32 *Key, bio::u32 *TagLocation, bio::u32 SelfTag, bio::u64 Timeout);
    bio::u32 __bio_svc_SignalProcessWideKey(bio::u32 *Key, int Number);
    bio::u64 __bio_svc_GetSystemTick(void);
    bio::u32 __bio_svc_ConnectToNamedPort(bio::u32 *Out_IPCSession, const char *Name);
    bio::u32 __bio_svc_SendSyncRequest(bio::u32 IPCSession);
    bio::u32 __bio_svc_SendSyncRequestWithUserBuffer(void *UserBuffer, bio::u64 Size, bio::u32 IPCSession);
    bio::u32 __bio_svc_SendAsyncRequestWithUserBuffer(bio::u32 *Handle, void *UserBuffer, bio::u64 Size, bio::u32 IPCSession);
    bio::u32 __bio_svc_GetProcessId(bio::u64 *Out_ProcessID, bio::u32 VarHandle);
    bio::u32 __bio_svc_GetThreadId(bio::u64 *Out_ThreadID, bio::u32 VarHandle);
    bio::u32 __bio_svc_Break(bio::u32 BreakReason, bio::u64 Param1, bio::u64 Param2);
    bio::u32 __bio_svc_OutputDebugString(const char *String, bio::u64 Size);
    void BIO_NORETURN __bio_svc_ReturnFromException(bio::u32 Code);
    bio::u32 __bio_svc_GetInfo(bio::u64 *Out_Info, bio::u64 FirstID, bio::u32 VarHandle, bio::u64 SecondID);
    bio::u32 __bio_svc_MapPhysicalMemory(void *Address, bio::u64 Size);
    bio::u32 __bio_svc_UnmapPhysicalMemory(void *Address, bio::u64 Size);
    // bio::u32 __bio_svc_GetResourceLimitLimitValue(bio::u64 *Out_Value, bio::u32 ResourceLimit, bio::svc::LimitableResource Which);
    // bio::u32 __bio_svc_GetResourceLimitCurrentValue(bio::u64 *Out_Value, bio::u32 ResourceLimit, bio::svc::LimitableResource Which);
    bio::u32 __bio_svc_SetThreadActivity(bio::u32 ThreadHandle, bool Pause);
    // bio::u32 __bio_svc_GetThreadContext3(bio::svc::ThreadContext *Context, bio::u32 ThreadHandle);
    bio::u32 __bio_svc_CreateSession(bio::u32 *ServerHandle, bio::u32 *ClientHandle, bio::u32 Unknown0, bio::u64 Unknown1);
    bio::u32 __bio_svc_AcceptSession(bio::u32 *SessionHandle, bio::u32 PortHandle);
    bio::u32 __bio_svc_ReplyAndReceive(int *Index, const bio::u32 *Handles, int HandleCount, bio::u32 ReplyTarget, bio::u64 Timeout);
    bio::u32 __bio_svc_ReplyAndReceiveWithUserBuffer(int *Index, void *UserBuffer, bio::u64 Size, const bio::u32 *Handles, int HandleCount, bio::u32 ReplyTarget, bio::u64 Timeout);
    bio::u32 __bio_svc_CreateEvent(bio::u32 *ServerHandle, bio::u32 *ClientHandle);
    bio::u32 __bio_svc_MapPhysicalMemoryUnsafe(void *Address, bio::u64 Size);
    bio::u32 __bio_svc_UnmapPhysicalMemoryUnsafe(void *Address, bio::u64 Size);
    bio::u32 __bio_svc_SetUnsafeLimit(bio::u64 Size);
    bio::u32 __bio_svc_CreateCodeMemory(bio::u32 *Out_CodeHandle, void *SourceAddress, bio::u64 Size);
    // bio::u32 __bio_svc_ControlCodeMemory(bio::u32 CodeHandle, bio::svc::CodeMapOperation MapOperation, void *DestAddress, bio::u64 Size, bio::u64 Permissions);
    bio::u32 __bio_svc_ReadWriteRegister(bio::u32 *Out_Value, bio::u64 RegisterAddress, bio::u32 ReadWriteMask, bio::u32 Value);
    bio::u32 __bio_svc_CreateSharedMemory(bio::u32 *Out_Handle, size_t Size, bio::u32 LocalPermissions, bio::u32 OtherPermissions);
    bio::u32 __bio_svc_MapTransferMemory(bio::u32 TransferMemory, void *Address, size_t Size, bio::u32 Permissions);
    bio::u32 __bio_svc_UnmapTransferMemory(bio::u32 TransferMemory, void *Address, size_t Size);
    bio::u32 __bio_svc_CreateInterruptEvent(bio::u32 *Out_Event, bio::u64 IRQNumber, bio::u32 Flags);
    bio::u32 __bio_svc_QueryPhysicalAddress(bio::u64 Out_Information[3], bio::u64 VirtualAddress);
    bio::u32 __bio_svc_QueryIoMapping(bio::u64 *Out_VirtualAddress, bio::u64 PhysicalAddress, bio::u64 Size);
    bio::u32 __bio_svc_CreateDeviceAddressSpace(bio::u32 *Out_AddressSpace, bio::u64 DevAddress, bio::u64 DevSize);
    bio::u32 __bio_svc_AttachDeviceAddressSpace(bio::u64 Device, bio::u32 AddressSpace);
    bio::u32 __bio_svc_DetachDeviceAddressSpace(bio::u64 Device, bio::u32 AddressSpace);
    bio::u32 __bio_svc_MapDeviceAddressSpaceByForce(bio::u32 DeviceAddress, bio::u32 ProcessHandle, bio::u64 MapAddress, bio::u64 DevSize, bio::u64 DevAddress, bio::u32 Permissions);
    bio::u32 __bio_svc_MapDeviceAddressSpaceAligned(bio::u32 DeviceAddress, bio::u32 ProcessHandle, bio::u64 MapAddress, bio::u64 DevSize, bio::u64 DevAddress, bio::u32 Permissions);
    bio::u32 __bio_svc_UnmapDeviceAddressSpace(bio::u32 DeviceAddress, bio::u32 ProcessHandle, bio::u64 MapAddress, bio::u64 MapSize, bio::u64 DevAddress);
    bio::u32 __bio_svc_DebugActiveProcess(bio::u32 *ProcessHandle, bio::u64 ProcessID);
    bio::u32 __bio_svc_BreakDebugProcess(bio::u32 ProcessHandle);
    bio::u32 __bio_svc_GetDebugEvent(bio::u8 *Out_Event, bio::u32 ProcessHandle);
    bio::u32 __bio_svc_ContinueDebugEvent(bio::u32 ProcessHandle, bio::u32 Flags, bio::u64 *TitleIDs, bio::u32 TitleIDCount);
    bio::u32 __bio_svc_LegacyContinueDebugEvent(bio::u32 ProcessHandle, bio::u32 Flags, bio::u64 ThreadID);
    bio::u32 __bio_svc_GetProcessList(bio::u32 *Out_ProcessCount, bio::u64 *Out_ProcessIDs, bio::u32 MaxProcessIDs);
    bio::u32 __bio_svc_GetThreadList(bio::u32 *Out_ThreadCount, bio::u64 *Out_TitleIDs, bio::u32 MaxTitleIDs, bio::u32 ProcessHandle);
    // bio::u32 __bio_svc_GetDebugThreadContext(bio::svc::ThreadContext *Out_Context, bio::u32 ProcessHandle, bio::u64 ThreadID, bio::u32 Flags);
    // bio::u32 __bio_svc_SetDebugThreadContext(bio::u32 ProcessHandle, bio::u64 ThreadID, const bio::svc::ThreadContext *Out_Context, bio::u32 Flags);
    bio::u32 __bio_svc_QueryDebugProcessMemory(bio::svc::MemoryInfo *MemInfo, bio::u32 *PageInfo, bio::u32 ProcessHandle, bio::u64 Address);
    bio::u32 __bio_svc_ReadDebugProcessMemory(void *Out_Buffer, bio::u32 ProcessHandle, bio::u64 Address, bio::u64 Size);
    bio::u32 __bio_svc_WriteDebugProcessMemory(bio::u32 ProcessHandle, void *Buffer, bio::u64 Address, bio::u64 Size);
    // bio::u32 __bio_svc_GetDebugThreadParam(bio::u64 *Out_64Bit, bio::u32 *Out_32Bit, bio::u32 ProcessHandle, bio::u64 ThreadID, bio::svc::DebugThreadParameter ThreadParam);
    bio::u32 __bio_svc_GetSystemInfo(bio::u64 *Out_Info, bio::u64 FirstID, bio::u32 VarHandle, bio::u64 SecondID);
    bio::u32 __bio_svc_CreatePort(bio::u32 *PortHandle, bio::u32 *ClientHandle, int MaxSessions, bool IsLight, const char *Name);
    bio::u32 __bio_svc_ManageNamedPort(bio::u32 *PortHandle, const char *Name, int MaxSessions);
    bio::u32 __bio_svc_ConnectToPort(bio::u32 *IPCSession, bio::u32 PortHandle);
    bio::u32 __bio_svc_SetProcessMemoryPermission(bio::u32 ProcessHandle, bio::u64 Address, bio::u64 Size, bio::u32 Permissions);
    bio::u32 __bio_svc_MapProcessMemory(void *DestAddress, bio::u32 ProcessHandle, bio::u64 SourceAddress, bio::u64 Size);
    bio::u32 __bio_svc_UnmapProcessMemory(void *DestAddress, bio::u32 ProcessHandle, bio::u64 SourceAddress, bio::u64 Size);
    bio::u32 __bio_svc_MapProcessCodeMemory(bio::u32 ProcessHandle, bio::u64 DestAddress, bio::u64 SourceAddress, bio::u64 Size);
    bio::u32 __bio_svc_UnmapProcessCodeMemory(bio::u32 ProcessHandle, bio::u64 DestAddress, bio::u64 SourceAddress, bio::u64 Size);
    bio::u32 __bio_svc_CreateProcess(bio::u32 *Out_Process, void *ProcessInfo, bio::u32 *Caps, bio::u64 CapCount);
    bio::u32 __bio_svc_StartProcess(bio::u32 ProcessHandle, int MainPriority, int DefaultCPU, bio::u32 StackSize);
    bio::u32 __bio_svc_TerminateProcess(bio::u32 ProcessHandle);
    // bio::u32 __bio_svc_GetProcessInfo(bio::u64 *Out_Value, bio::u32 ProcessHandle, bio::svc::ProcessInfo InfoType);
    bio::u32 __bio_svc_CreateResourceLimit(bio::u32 *ResourceLimit);
    // bio::u32 __bio_svc_SetResourceLimitLimitValue(bio::u32 ResourceLimit, bio::svc::LimitableResource Resource, bio::u64 Value);
    bio::u64 __bio_svc_CallSecureMonitor(bio::svc::SecureMonitorArgs *Args);
}

namespace bio::svc
{
    Result CloseHandle(u32 handle)
    {
        return __bio_svc_CloseHandle(handle);
    }

    Result CreateEvent(Out<u32> w_end, Out<u32> r_end)
    {
        return __bio_svc_CreateEvent(w_end.AsPtr(), r_end.AsPtr());
    }
    
    Result ResetSignal(u32 signal)
    {
        return __bio_svc_ResetSignal(signal);
    }
    
    Result WaitSynchronization(Out<u32> handle_index, u32 *handles, u32 num_handles, u64 timeout)
    {
        return __bio_svc_WaitSynchronization(handle_index.AsPtr(), handles, num_handles, timeout);
    }
    
    Result SignalEvent(u32 event)
    {
        return __bio_svc_SignalEvent(event);
    }
    
    Result CreateTransferMemory(Out<u32> handle, void *addr, u64 size, u32 permission)
    {
        return __bio_svc_CreateTransferMemory(handle.AsPtr(), addr, size, permission);
    }
    
    Result SendSyncRequest(u32 handle)
    {
        return __bio_svc_SendSyncRequest(handle);
    }
    
    Result ConnectToNamedPort(Out<u32> session_handle, const char *name)
    {
        return __bio_svc_ConnectToNamedPort(session_handle.AsPtr(), name);
    }

    Result SleepThread(i64 nanoseconds)
    {
        return __bio_svc_SleepThread(nanoseconds);
    }

    Result OutputDebugString(char *str, bio::u64 size)
    {
        return __bio_svc_OutputDebugString(str, size);
    }

    Result SetHeapSize(Out<void*> address, u64 size)
    {
        return __bio_svc_SetHeapSize(address.AsPtr(), size);
    }

    void ExitProcess()
    {
        __bio_svc_ExitProcess();
    }

    Result GetInfo(u32 first_id, u32 second_id, u32 handle, Out<u64> info)
    {
        return __bio_svc_GetInfo(info.AsPtr(), first_id, handle, second_id);
    }
}