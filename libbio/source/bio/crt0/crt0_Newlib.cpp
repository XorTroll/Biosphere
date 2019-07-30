#include <sys/reent.h>
#include <unistd.h>
#include <errno.h>
#include <bio/svc/svc_Base.hpp>
#include <cstring>
#include <cstdio>
#include <pthread.h>
#include <cstdlib>
#include <cstdint>
#include <cstdbool>
#include <errno.h>
#include <bio/os/os_TLS.hpp>
#include <malloc.h>
#include <bio/log/log_Logging.hpp>
#include <bio/svc/svc_Base.hpp>
#include <bio/os/os_Memory.hpp>
#include <math.h>

extern "C"
{
    typedef bool _Bool;

    #include <stdatomic.h>
    #include <phal.h>
}

extern "C"
{
    long double nanl(const char *s) // Why isn't this defined...?
    {
        return NAN;
    }

    void _exit(int ret)
    {
        bio::svc::ExitProcess();
    }

    int _getpid_r(struct _reent *reent)
    {
        reent->_errno = ENOSYS;
        return -1;
    }

    int _kill_r(struct _reent *reent, int pid, int sig)
    {
        reent->_errno = ENOSYS;
        return -1;
    }

    int nanosleep(const struct timespec *rqtp, struct timespec *rmtp)
    {
	    auto res = bio::svc::SleepThread(rqtp->tv_nsec + (rqtp->tv_sec * 1'000'000'000));
        if(res.IsFailure())
        {
            errno = bio::Result::GetErrnoFrom(res);
            return -1;
        }
        return res;
    }

    long sysconf(int name)
    {
        switch(name)
        {
            case _SC_PAGESIZE:
                return 0x1000;
        }
        errno = ENOSYS;
	    return -1;
    }

    void *global_HeapAddress = NULL;
    size_t global_HeapSize = 0;

    bio::os::VirtualRegion global_AddressSpace;
    bio::os::VirtualRegion global_Regions[4];

    static size_t heap_capacity = 0;
    static const size_t heap_incr_multiple = 0x200000;

    void *_sbrk_r(struct _reent *reent, ptrdiff_t incr)
    {
        if(global_HeapSize + incr > heap_capacity)
        {
            ptrdiff_t capacity_incr = global_HeapSize + incr - heap_capacity;
            capacity_incr += (heap_incr_multiple - 1);
            capacity_incr = capacity_incr - (capacity_incr % heap_incr_multiple);
            auto res = bio::svc::SetHeapSize(global_HeapAddress, heap_capacity + capacity_incr);
            if(res.IsSuccess()) heap_capacity += capacity_incr;
            else return NULL;
        }
        void *addr = (bio::u8*)global_HeapAddress + global_HeapSize;
        global_HeapSize += incr;
        return addr;
    }

    int posix_memalign (void **memptr, size_t alignment, size_t size)
    {
        if(alignment % sizeof(void*) != 0 || (alignment & (alignment - 1)) != 0) return EINVAL;

        void *mem = memalign(alignment, size);
        
        if(mem != NULL)
        {
            *memptr = mem;
            return 0;
        }
        
        return ENOMEM;
    }

    int phal_thread_create(phal_tid *tid, void (*start_routine)(void*), void *arg)
    {
        return ENOSYS;
    }

    int sched_yield()
    {
        auto res = bio::svc::SleepThread(0);
        if(res.IsFailure())
        {
            errno = bio::Result::GetErrnoFrom(res);
            return -1;
        }
        return res;
    }


    void phal_thread_exit(phal_tid *tid)
    {
        // bio::svc::ExitThread();
    }

    int phal_thread_destroy(phal_tid *tid)
    {
        errno = ENOSYS;
        return -1;
    }

    int phal_thread_sleep(uint64_t msec)
    {
        auto res = bio::svc::SleepThread(msec * 1000 * 1000);
        if(res.IsFailure())
        {
            errno = bio::Result::GetErrnoFrom(res);
            return -1;
        }
        return res;
    }

    int phal_semaphore_create(phal_semaphore *sem)
    {
        sem->lock = 0;
        sem->sem = 0;
        return 0;
    }

    int phal_semaphore_destroy(phal_semaphore *sem)
    {
        return 0;
    }

    /// Wake up one thread waiting for the semaphore.
    int phal_semaphore_signal(phal_semaphore *sem)
    {
        errno = ENOSYS;
        return -1;
    }

    int phal_semaphore_broadcast(phal_semaphore *sem)
    {
        errno = ENOSYS;
        return -1;
    }

    // TODO: This kinda sucks. Different platform behave differently here. For now
    // let's focus on the switch, which needs a locked mutex and a semaphore.
    /// Wait for the semaphore to be signaled. Note that we should **not** wake up
    /// if a signal was previously sent. This is not a counting semaphore.
    int phal_semaphore_wait(phal_semaphore *sem, const struct timespec *abstime)
    {
        errno = ENOSYS;
        return -1;
    }

    /*
    int phal_mutex_create(phal_mutex *mutex)
    {
        *mutex = 0;
        return 0;
    }

    int phal_mutex_destroy(phal_mutex *mutex)
    {
        return 0;
    }
    */

    int phal_semaphore_lock(phal_semaphore *sem)
    {
        errno = ENOSYS;
        return -1;
    }

    int phal_semaphore_unlock(phal_semaphore *sem)
    {
        errno = ENOSYS;
        return -1;
    }

    void **phal_get_tls()
    {
        return NULL; // F I X !!!
    }

    int _fstat_r(struct _reent *reent, int file, struct stat *st)
    {
        reent->_errno = ENOSYS;
        return -1;
    }

    int _isatty_r(struct _reent *reent, int file)
    {
        reent->_errno = ENOSYS;
        return -1;
    }

    int _gettimeofday_r(struct _reent *reent, struct timeval *__restrict p, void *__restrict z)
    {
        reent->_errno = EINVAL;
        return -1;
    }

    int clock_gettime(clockid_t clk_id, struct timespec *tp)
    {
        errno = ENOSYS;
        return -1;
    }

    int clock_settime(clockid_t clk_id, const struct timespec *tp)
    {
        errno = ENOSYS;
        return -1;
    }

    int _rename_r(struct _reent *reent, const char *r_old, const char *r_new)
    {
        // TODO: implement this
        reent->_errno = EROFS;
        return -1;
    }

    int _unlink_r(struct _reent *reent, const char *name)
    {
        reent->_errno = EBADF;
        return -1;
    }

    int tcgetattr(int fd, struct termios *termios_p)
    {
        errno = ENOSYS;
        return -1;
    }

    int tcsetattr(int fd, int optional_actions, const struct termios *termios_p)
    {
        errno = ENOSYS;
        return -1;
    }

    int ioctl(int fd, unsigned long request, ...)
    {
        errno = ENOSYS;
        return -1;
    }

    int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout)
    {
        errno = ENOSYS;
        return -1;
    }
}