

#pragma once
#include <bio/svc/svc_Base.hpp>
#include <bio/bio_Kernel.hpp>

namespace bio::os
{
    enum class Region
    {
        Stack = 0,
        Heap,
        NewStack,
        Max
    };

    struct VirtualRegion
    {
        u64 start;
        u64 end;
    };

    class SharedMemory
    {
        public:
            SharedMemory(size_t size, Permission local, Permission remote);
            SharedMemory(u32 handle, size_t size, Permission permissions);
            ~SharedMemory();
            Result Map();
            Result Unmap();
            void *GetAddress();
        private:
            u32 handle;
            size_t size;
            Permission perms;
            void *address;
    };

    class TransferMemory
    {
        public:
            TransferMemory(size_t size, Permission permissions);
            TransferMemory(u32 handle, size_t size, Permission permissions);
            ~TransferMemory();
            Result Map();
            Result Unmap();
            void *GetAddress();
        private:
            u32 handle;
            size_t size;
            Permission perms;
            void *backaddress;
            void *mapaddress;
    };

    Result ReserveVirtualMemory(Out<void*> address, size_t size);
    void FreeVirtualMemory(void *address, size_t size);
    Result ReserveVirtualMemoryMap(Out<void*> address, size_t size);
    void FreeVirtualMemoryMap(void *address, size_t size);
}