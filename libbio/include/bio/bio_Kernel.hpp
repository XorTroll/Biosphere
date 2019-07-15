
#pragma once
#include <bio_Types.hpp>

namespace bio
{
    class KObject
    {
        public:
            KObject();
            KObject(u32 handle);
            KObject(const KObject &) = delete;
            KObject &operator=(const KObject &) = delete;
            KObject(KObject &&other);
            KObject &operator=(KObject &&other);
            ~KObject();

            u32 Claim();
            
            u32 handle = 0;
    };

    static KObject InvalidObject = KObject(0);

    class KWaitable : public KObject
    {
        public:
            KWaitable() = default;
            KWaitable(u32 handle);
    };

    class KSharedMemory : public KObject
    {
        public:
            KSharedMemory() = default;

            // foreign shared memory
            KSharedMemory(u32 handle, size_t size, uint32_t foreign_permission);

            size_t size;
            uint32_t foreign_permission;
    };

    class KTransferMemory : public KObject
    {
        public:
            KTransferMemory() = default;
            
            // foreign transfer memory
            KTransferMemory(u32 handle, size_t size, uint32_t permissions);

            // owned transfer memory
            KTransferMemory(u32 handle, void *backing_buffer, size_t size, uint32_t permissions);
            KTransferMemory(size_t size, uint32_t permissions);
            KTransferMemory(void *backing_buffer, size_t size, uint32_t permissions, bool owns_buffer=false);

            // misc
            KTransferMemory(KTransferMemory &&other);
            KTransferMemory &operator=(KTransferMemory &&other);
            ~KTransferMemory();

            uint8_t *buffer = nullptr;
            size_t size = 0;
            uint32_t permissions = 0;
        private:
            bool owns_buffer = false;
    };

    class KPort : public KWaitable
    {
        public:
            KPort() = default;
            KPort(u32 handle);
    };

    class KProcess : public KWaitable
    {
        public:
            KProcess() = default;
            KProcess(u32 handle);
            Result ResetSignal();
            Result WaitSignal(uint64_t timeout);
    };

    class KEvent : public KWaitable
    {
        public:
            KEvent() = default;
            KEvent(u32 handle);
            Result ResetSignal();
            Result WaitSignal(uint64_t timeout);
    };

    // Writable event
    class KWEvent : public KObject
    {
        public:
            KWEvent() = default;
            KWEvent(KEvent &read_end); // creates a new event
            KWEvent(u32 handle);
            Result Signal();
    };

    class KDebug : public KWaitable
    {
        public:
            KDebug() = default;
            KDebug(u32 handle);
    };

    class KResourceLimit : public KObject
    {
        public:
            KResourceLimit() = default;
            KResourceLimit(u32 handle);
    };
}