
#pragma once
#include <bio/bio_Types.hpp>
#include <sys/reent.h>

namespace bio::os
{
    typedef void(*ThreadEntrypoint)(void *args);

    struct ThreadBlock
    {
        u32 handle;
        bool owns_stack;
        void *stack;
        void *stack_mirror;
        size_t stack_size;
        ThreadEntrypoint entrypoint;
        void *arg;
        struct _reent reent;
        void **tls;
    };

    struct ThreadSection
    {
        u32 thread_handle;
        ThreadBlock *cur_thread;
        struct _reent *reent;
        void *tls_segment;
    };

    class Thread
    {
        public:
            ThreadBlock *GetBaseBlock();
    };

    ThreadSection *GetThreadSection();
}