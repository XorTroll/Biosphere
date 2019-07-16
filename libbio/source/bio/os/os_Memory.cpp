#include <bio/os/os_Memory.hpp>
#include <bio/os/os_Version.hpp>
#include <malloc.h>
#include <cstring>

extern bio::os::VirtualRegion global_AddressSpace;
extern bio::os::VirtualRegion global_Regions[4];
// bio::os::Mutex vmutex;
bio::u64 curaddr;
bio::u64 curmapaddr;

namespace bio::os
{
    Result ReserveVirtualMemory(Out<void*> address, size_t size)
    {
        svc::MemoryInfo meminfo;
        u32 pageinfo;
        u32 i = 0;
        size = (size + 0xfff) &~ 0xfff;
        // vmutex.Lock();
        u64 addr = curaddr;
        bio::Result res(0);
        while(true)
        {
            addr += 0x1000;
            if(!((addr >= global_AddressSpace.start) && (addr < global_AddressSpace.end))) addr = global_AddressSpace.start;
            res = svc::QueryMemory(addr, meminfo, pageinfo);
            if(res.IsFailure()) break;
            if(meminfo.type != 0)
            {
                addr = meminfo.address + meminfo.size;
                continue;
            }
            if(size > meminfo.size)
            {
                addr = meminfo.address + meminfo.size;
                continue;
            }
            for(i = 0; i < 3; i++)
            {
                u64 end = (addr + size - 1);
                if((addr >= global_Regions[i].start) && (addr < global_Regions[i].end)) break;
                if((end >= global_Regions[i].start) && (end < global_Regions[i].end)) break;
            }
            if(i != 3)
            {
                addr = global_Regions[i].end;
                continue;
            }
            break;
        }
        curaddr = addr + size;
        // vmutex.Unlock();
        if(res.IsSuccess()) (void*&)address = (void*)addr;
        return res;
    }

    void FreeVirtualMemory(void *address, size_t size)
    {
        BIO_IGNORE(address);
        BIO_IGNORE(size);
    }

    Result ReserveVirtualMemoryMap(Out<void*> address, size_t size)
    {
        u32 rc = 0;
        svc::MemoryInfo meminfo;
        u32 pageinfo = 0;
        auto ver = GetFirmwareVersion();
        int regidx = ((ver.major > 2) ? static_cast<int>(Region::NewStack) : static_cast<int>(Region::Stack));
        size = (size + 0xfff) &~ 0xfff;
        // vmutex.Lock();
        u64 addr = curaddr;
        bio::Result res(0);
        while(true)
        {
            addr += 0x1000;
            if(!((addr >= global_Regions[regidx].start) && (addr < global_Regions[regidx].end))) addr = global_Regions[regidx].start;
            res = svc::QueryMemory(addr, meminfo, pageinfo);
            if(res.IsFailure()) break;
            if(meminfo.type != 0)
            {
                addr = (meminfo.address + meminfo.size);
                continue;
            }
            if(size > meminfo.size)
            {
                addr = (meminfo.address + meminfo.size);
                continue;
            }
            break;
        }
        curaddr = (addr + size);
        //vmutex.Unlock();
        if(res.IsSuccess()) (void*&)address = (void*)addr;
        return res;
    }

    void FreeMapVirtualMemory(void *address, size_t size)
    {
        BIO_IGNORE(address);
        BIO_IGNORE(size);
    }

    SharedMemory::SharedMemory(size_t size, Permission local, Permission remote) : handle(0), size(size), perms(local), address(NULL)
    {
        svc::CreateSharedMemory(handle, size, static_cast<u32>(local), static_cast<u32>(remote));
    }

    SharedMemory::SharedMemory(u32 handle, size_t size, Permission permissions) : handle(handle), size(size), address(NULL), perms(permissions)
    {
    }

    SharedMemory::~SharedMemory()
    {
        u32 rc = 0;
        if(address != NULL) rc = Unmap();
        if(rc == 0)
        {
            if(handle != 0) rc = svc::CloseHandle(handle);
            handle = 0;
        }
    }

    Result SharedMemory::Map()
    {
        bio::Result res(0);
        if(address == NULL)
        {
            void *addr = NULL;
            res = ReserveVirtualMemory(addr, size);
            if(res.IsSuccess())
            {
                res = svc::MapSharedMemory(handle, addr, size, static_cast<u32>(perms));
                if(res.IsSuccess()) address = addr;
                else FreeVirtualMemory(addr, size);
            }
        }
        return res;
    }

    Result SharedMemory::Unmap()
    {
        auto res = svc::UnmapSharedMemory(handle, address, size);
        if(res.IsSuccess())
        {
            FreeVirtualMemory(address, size);
            address = NULL;
        }
        return res;
    }

    void *SharedMemory::GetAddress()
    {
        return address;
    }

    TransferMemory::TransferMemory(size_t size, Permission permissions) : handle(0), size(size), perms(permissions), mapaddress(NULL), backaddress(memalign(0x1000, size))
    {
        bio::Result res(0);
        if(backaddress != NULL)
        {
            memset(backaddress, 0, size);
            res = svc::CreateTransferMemory(handle, backaddress, size, static_cast<u32>(perms));
        }
        if(res.IsFailure())
        {
            free(backaddress);
            backaddress = NULL;
        }
    }

    TransferMemory::TransferMemory(u32 Handle, size_t Size, Permission Permissions)
    {
        handle = Handle;
        size = Size;
        perms = Permissions;
        mapaddress = NULL;
        backaddress = NULL;
    }

    TransferMemory::~TransferMemory()
    {
        u32 rc = 0;
        if(mapaddress != NULL) rc = Unmap();
        if(rc == 0)
        {
            if(handle != 0) rc = svc::CloseHandle(handle);
            handle = 0;
        }
        if(backaddress != NULL)
        {
            free(backaddress);
            backaddress = NULL;
        }
    }

    Result TransferMemory::Map()
    {
        bio::Result res(0);
        if(mapaddress == NULL)
        {
            void *addr;
            res = ReserveVirtualMemory(addr, size);
            if(res.IsSuccess())
            {
                res = svc::MapTransferMemory(handle, addr, size, static_cast<u32>(perms));
                if(res.IsSuccess()) mapaddress = addr;
                else FreeVirtualMemory(addr, size);
            }
        }
        return res;
    }

    Result TransferMemory::Unmap()
    {
        u32 rc = svc::UnmapTransferMemory(handle, mapaddress, size);
        if(rc == 0)
        {
            FreeVirtualMemory(mapaddress, size);
            mapaddress = NULL;
        }
        return rc;
    }

    void *TransferMemory::GetAddress()
    {
        return mapaddress;
    }
}