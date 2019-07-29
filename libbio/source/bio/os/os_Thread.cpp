#include <bio/os/os_Thread.hpp>
#include <bio/os/os_TLS.hpp>
#include <bio/svc/svc_Base.hpp>
#include <cstring>
#include <malloc.h>

namespace bio::os
{
    static void _inner_ThreadEntryWrap(void *thread_raw)
    {
        auto thread = (ThreadBlock*)thread_raw;
        GetThreadSection()->thread = thread;
        _REENT_INIT_PTR(&thread->reent);
        thread->entrypoint(thread->arg);
        svc::ExitThread();
    }

    static Result _inner_CreateThreadBlock(ThreadBlock *block, ThreadEntrypoint entry, void *entry_arg, size_t stack_size, u32 priority, i32 cpu_id, const char *name)
    {
        memset(block, 0, sizeof(ThreadBlock));
        if(priority == -1) svc::GetThreadPriority(svc::CurrentThreadPseudoHandle, priority);
        block->owns_stack = true;
        block->stack = operator new(stack_size);
        block->stack_size = stack_size;
        block->entrypoint = entry;
        block->arg = entry_arg;
        strcpy(block->name, name);
        block->pthread = NULL;
        return svc::CreateThread((void*)_inner_ThreadEntryWrap, (void*)block, (void*)((u8*)block->stack + stack_size), priority, cpu_id, block->handle);
    }

    Thread::Thread(ThreadBlock *raw_thread) : raw(raw_thread)
    {
    }

    Thread::~Thread()
    {
        operator delete(raw->stack);
        svc::CloseHandle(raw->handle);
        delete raw;
    }

    Result Thread::Create(ThreadEntrypoint entry, void *entry_arg, size_t stack_size, u32 priority, const char *name, Out<std::shared_ptr<Thread>> out)
    {
        ThreadBlock *block = new ThreadBlock;
        auto res = _inner_CreateThreadBlock(block, entry, entry_arg, stack_size, priority, -2, name);
        if(res.IsSuccess()) (std::shared_ptr<Thread>&)out = std::make_shared<Thread>(block);
        return res;
    }

    Result Thread::Start()
    {
        return svc::StartThread(raw->handle);
    }

    ThreadSection *GetThreadSection()
    {
        return (ThreadSection*)GetTLS();
    }

    ThreadBlock *GetCurrentThreadBlock()
    {
        return GetThreadSection()->thread;
    }

    void SetCurrentThreadName(const char *name)
    {
        strcpy(GetThreadSection()->thread->name, name);
    }

    u32 GetCurrentThreadHandle()
    {
        auto thread = GetCurrentThreadBlock();
        if(thread != NULL) return thread->handle;
        return 0xFFFFFFFF;
    }
}