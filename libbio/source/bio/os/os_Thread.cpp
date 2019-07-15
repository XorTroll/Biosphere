#include <bio/os/os_Thread.hpp>
#include <bio/os/os_TLS.hpp>

namespace bio::os
{
    ThreadSection *GetThreadSection()
    {
        return (ThreadSection*)((u8*)GetTLS() + 0x200 - sizeof(ThreadSection));
    }

    struct _inner_ThreadStartData
    {
        ThreadBlock *thread;
        ThreadEntrypoint entry;
        void *entry_arg;
        struct _reent *reent;
        void *tls;
        void *pad;
    };

    static void _inner_ThreadStartBase(_inner_ThreadStartData *data)
    {
        auto section = GetThreadSection();
        section->cur_thread = data->thread;
        section->reent = data->reent;
        section->tls_segment = (u8*)data->tls - (2 * sizeof(void*));
        section->thread_handle = data->thread->handle;
        data->thread->tls = (void**)((u8*)GetTLS() + 0x108);

        data->entry(data->entry_arg);

        // exit
    }
}