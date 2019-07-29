
#pragma once
#include <bio/bio_Types.hpp>
#include <sys/reent.h>
#include <memory>

namespace bio::os
{
    typedef void(*ThreadEntrypoint)(void *args);

    struct ThreadBlock
    {
        u32 handle;
        bool owns_stack;
        void *stack;
        size_t stack_size;
        ThreadEntrypoint entrypoint;
        void *arg;
        void *pthread;
        u8 zero[0x158]; // Space for thread name
        char name[0x20];
        void *nameptr;
        struct _reent reent;
    };

    struct ThreadSection
    {
        u32 ipc_buffer[0x40];
        u8 unk[0xF8];
        ThreadBlock *thread;
    };
    
    class Thread
    {
        public:
            Thread(ThreadBlock *raw_thread);
            ~Thread();
            static ResultWith<std::shared_ptr<Thread>> Create(ThreadEntrypoint entry, void *entry_arg, size_t stack_size, u32 priority, const char *name);
            Result Start();
            Result Join();
        private:
            ThreadBlock *raw;
    };

    ThreadSection *GetThreadSection();

    ThreadBlock *GetCurrentThreadBlock();
    void SetCurrentThreadName(const char *name);
    u32 GetCurrentThreadHandle();
}