
#pragma once
#include <bio/bio_Types.hpp>
#include <sys/lock.h>
#include <mutex>

namespace bio::os
{
    class Mutex
    {
        public:
            Mutex();
            void Lock();
            bool TryLock();
            void Unlock();
            _LOCK_T *GetNativeLock();
        private:
            _LOCK_T mutex;
    };

    class RecursiveMutex
    {
        public:
            RecursiveMutex();
            void Lock();
            bool TryLock();
            void Unlock();
            _LOCK_T *GetNativeLock();
        private:
            _LOCK_T mutex;
            u32 handle;
            u32 counter;
    };

    class ConditionVariable
    {
        public:
            ConditionVariable();
            Result WaitTimeout(Mutex &wait_mutex, u64 timeout);
            Result Wait(Mutex &wait_mutex);
            Result Wake(i32 thread_no);
            Result WakeOne();
            Result WakeAll();
        private:
            u32 cvar;
    };
}