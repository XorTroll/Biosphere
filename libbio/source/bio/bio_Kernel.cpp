#include <bio_Kernel.hpp>
#include <bio/svc/svc_Base.hpp>
#include <utility>

namespace bio
{
    KObject::KObject() : handle(0)
    {
    }

    KObject::KObject(u32 handle) : handle(handle)
    {
    }

    KObject::KObject(KObject &&other)
    {
        this->handle = other.handle;
        other.handle = 0;
    }

    KObject &KObject::operator=(KObject &&other)
    {
        this->handle = other.handle;
        other.handle = 0;
        return *this;
    }

    KObject::~KObject()
    {
        if(handle > 0)
        {
            svc::CloseHandle(handle).Assert();
        }
    }

    u32 KObject::Claim()
    {
        u32 handle = this->handle;
        this->handle = 0;
        return handle;
    }

    KWaitable::KWaitable(u32 handle) : KObject(handle)
    {
    }

    KSharedMemory::KSharedMemory(u32 handle, size_t size, uint32_t foreign_permission) : KObject(handle), size(size), foreign_permission(foreign_permission)
    {
    }

    KTransferMemory::KTransferMemory(u32 handle, size_t size, uint32_t permissions) : KObject(handle), size(size), permissions(permissions)
    {
    }

    KTransferMemory::KTransferMemory(u32 handle, void *addr, size_t size, uint32_t permissions) : KObject(handle), buffer((uint8_t*) addr), size(size), permissions(permissions)
    {
    }

    KTransferMemory::KTransferMemory(size_t size, uint32_t permissions) : KTransferMemory(malloc(size), size, permissions, true)
    {
    }

    KTransferMemory::KTransferMemory(void *buffer, size_t size, uint32_t permissions, bool owns_buffer)
    {
        svc::CreateTransferMemory(handle, buffer, size, permissions).Assert();
        this->owns_buffer = owns_buffer;
    }

    KTransferMemory::KTransferMemory(KTransferMemory &&other) : KObject(std::move(other)), buffer(other.buffer), size(other.size), permissions(other.permissions), owns_buffer(other.owns_buffer)
    {
        other.owns_buffer = false;
    }

    KTransferMemory &KTransferMemory::operator=(KTransferMemory &&other)
    {
        buffer = other.buffer;
        size = other.size;
        permissions = other.permissions;
        owns_buffer = other.owns_buffer;
        other.owns_buffer = false;
        KObject::operator=(std::move(other));
        return *this;
    }

    KTransferMemory::~KTransferMemory()
    {
        if(owns_buffer && buffer)
        {
            free(buffer);
        }
    }

    KPort::KPort(u32 handle) : KWaitable(handle)
    {
    }

    KProcess::KProcess(u32 handle) : KWaitable(handle)
    {
    }

    KEvent::KEvent(u32 handle) : KWaitable(handle)
    {
    }

    KWEvent::KWEvent(u32 handle) : KObject(handle)
    {
    }

    KDebug::KDebug(u32 handle) : KWaitable(handle)
    {
    }

    KResourceLimit::KResourceLimit(u32 handle) : KObject(handle)
    {
    }

    KWEvent::KWEvent(KEvent &read_end)
    {
        KEvent read_end_intermediate;
        svc::CreateEvent(handle, read_end_intermediate.handle).Assert();
        read_end = std::move(read_end_intermediate);
    }

    Result KProcess::ResetSignal()
    {
        return svc::ResetSignal(handle);
    }

    Result KProcess::WaitSignal(u64 timeout)
    {
        uint32_t handle_index;
        return svc::WaitSynchronization(handle_index, &handle, 1, timeout);
    }

    Result KEvent::ResetSignal()
    {
        return svc::ResetSignal(handle);
    }

    Result KEvent::WaitSignal(u64 timeout)
    {
        uint32_t handle_index;
        return svc::WaitSynchronization(handle_index, &handle, 1, timeout);
    }

    Result KWEvent::Signal()
    {
        return svc::SignalEvent(handle);
    }
}