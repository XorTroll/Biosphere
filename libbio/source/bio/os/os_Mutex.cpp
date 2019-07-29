#include <bio/os/os_Mutex.hpp>
#include <bio/os/os_Thread.hpp>
#include <bio/svc/svc_Base.hpp>

namespace bio::os
{
    Mutex::Mutex()
    {
        this->mutex = 0;
    }

    void Mutex::Lock()
    {
        u32 self = GetCurrentThreadHandle();
        while(true)
        {
            u32 cur = __sync_val_compare_and_swap((u32*)&this->mutex, 0, self);
            if(cur == 0) return;
            if((cur &~ 0x40000000) == self) return;
            if(cur & 0x40000000) svc::ArbitrateLock((cur &~ 0x40000000), (u32&)this->mutex, self);
            else
            {
                u32 old = __sync_val_compare_and_swap((u32*)&this->mutex, cur, (cur | 0x40000000));
                if(old == cur) svc::ArbitrateLock(cur, (u32&)this->mutex, self);
            }
        }
    }

    bool Mutex::TryLock()
    {
        u32 self = GetCurrentThreadHandle();
        u32 cur = __sync_val_compare_and_swap((u32*)&this->mutex, 0, self);
        if(cur == 0) return true;
        if((cur &~ 0x40000000) == self) return true;
        return false;
    }

    void Mutex::Unlock()
    {
        u32 old = __sync_val_compare_and_swap((u32*)&this->mutex, GetCurrentThreadHandle(), 0);
        if(old & 0x40000000) svc::ArbitrateUnlock((u32&)this->mutex);
    }

    _LOCK_T *Mutex::GetNativeLock()
    {
        return &this->mutex;
    }

    RecursiveMutex::RecursiveMutex()
    {
        this->mutex = 0;
        this->handle = 0;
        this->counter = 0;
    }

    void RecursiveMutex::Lock()
    {
        if(this->handle != GetCurrentThreadHandle())
        {
            u32 self = GetCurrentThreadHandle();
            while(true)
            {
                u32 cur = __sync_val_compare_and_swap((u32*)&this->mutex, 0, self);
                if(cur == 0) return;
                if((cur &~ 0x40000000) == self) return;
                if(cur & 0x40000000) svc::ArbitrateLock(cur &~ 0x40000000, (u32&)this->mutex, self);
                else
                {
                    u32 old = __sync_val_compare_and_swap((u32*)&this->mutex, cur, (cur | 0x40000000));
                    if(old == cur) svc::ArbitrateLock(cur, (u32&)this->mutex, self);
                }
            }
            this->handle = GetCurrentThreadHandle();
        }
        this->counter++;
    }

    bool RecursiveMutex::TryLock()
    {
        if(this->handle != GetCurrentThreadHandle())
        {
            bool tlock = false;
            u32 self = GetCurrentThreadHandle();
            u32 cur = __sync_val_compare_and_swap((u32*)&this->mutex, 0, self);
            if(cur == 0) tlock = true;
            if((cur &~ 0x40000000) == self) tlock = true;
            if(!tlock) return false;
            this->handle = GetCurrentThreadHandle();
        }
        this->counter++;
        return true;
    }

    void RecursiveMutex::Unlock()
    {
        if(--this->counter == 0)
        {
            this->handle = 0;
            u32 old = __sync_val_compare_and_swap((u32*)&this->mutex, GetCurrentThreadHandle(), 0);
            if(old & 0x40000000) svc::ArbitrateUnlock((u32&)this->mutex);
        }
    }

    _LOCK_T *RecursiveMutex::GetNativeLock()
    {
        return &this->mutex;
    }

    ConditionVariable::ConditionVariable()
    {
        this->cvar = 0;
    }

    Result ConditionVariable::WaitTimeout(Mutex &wait_mutex, u64 timeout)
    {
        auto rc = svc::WaitProcessWideKeyAtomic((u32*)wait_mutex.GetNativeLock(), this->cvar, GetCurrentThreadHandle(), timeout);
        if(rc == 0xea01) wait_mutex.Lock();
        return rc;
    }

    Result ConditionVariable::Wait(Mutex &wait_mutex)
    {
        return this->WaitTimeout(wait_mutex, UINT64_MAX);
    }

    Result ConditionVariable::Wake(i32 thread_no)
    {
        return svc::SignalProcessWideKey(&this->cvar, thread_no);
    }

    Result ConditionVariable::WakeOne()
    {
        return this->Wake(1);
    }

    Result ConditionVariable::WakeAll()
    {
        return this->Wake(-1);
    }
}