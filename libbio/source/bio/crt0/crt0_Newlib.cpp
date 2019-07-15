#include <sys/reent.h>
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

extern bio::log::LogWriteFunction global_StdoutLog;

extern "C"
{
    typedef bool _Bool;

    #include <stdatomic.h>
    #include <phal.h>

    bio::u32 __bio_svc_SetHeapSize(void **OutAddress, bio::u64 Size);
}

extern "C"
{
    static int result_to_errno(bio::u32 res)
    {
        switch (res)
        {
            case 0: return 0;
            case 0xEA01: return ETIMEDOUT;
            default: return ENOSYS;
        }
    }

    int _getpid_r(struct _reent *reent)
    {
        reent->_errno = ENOSYS;
        return -1;
    }

    void _exit(int code)
    {
        // ??
    }

    int _kill_r(struct _reent *reent, int pid, int sig)
    {
        reent->_errno = ENOSYS;
        return -1;
    }

    ssize_t _write_r(struct _reent *reent, int file, const void *ptr, size_t len)
    {
        if(file == 1) // special case for stdout
        {
            global_StdoutLog(ptr, len);
            return len;
        }
        return -ENOSYS;
    }

    void *heap_addr = NULL;
    size_t heap_size = 0;
    static size_t heap_capacity = 0;
    static const size_t heap_incr_multiple = 0x200000;

    void *_sbrk_r(struct _reent *reent, ptrdiff_t incr)
    {

        if(heap_size + incr > heap_capacity)
        {
            ptrdiff_t capacity_incr = heap_size + incr - heap_capacity;
            capacity_incr += (heap_incr_multiple - 1);
            capacity_incr = capacity_incr - (capacity_incr % heap_incr_multiple);
            auto res = __bio_svc_SetHeapSize(&heap_addr, heap_capacity + capacity_incr);
            if(res == 0) heap_capacity += capacity_incr;
            else return NULL;
        }

        void *addr = (bio::u8*)heap_addr + heap_size;
        heap_size += incr;
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

    int phal_thread_create(phal_tid *tid, void (*start_routine)(void*), void *arg) {
        return ENOSYS;
    }

    int sched_yield() {
        return bio::svc::SleepThread(0) != 0 ? -1 : 0;
    }


    // Noreturn !
    void phal_thread_exit(phal_tid *tid) {
        //svcExitThread();
    }

    int phal_thread_destroy(phal_tid *tid) {
        return ENOSYS;
    }

    int phal_thread_sleep(uint64_t msec) {
        uint64_t nanos = msec * 1000 * 1000;
        return result_to_errno(bio::svc::SleepThread(nanos));
    }

    int phal_semaphore_create(phal_semaphore *sem) {
        sem->lock = 0;
        sem->sem = 0;
        return 0;
    }

    int phal_semaphore_destroy(phal_semaphore *sem) { return 0; }

    /// Wake up one thread waiting for the semaphore.
    int phal_semaphore_signal(phal_semaphore *sem) {
        return ENOSYS;
    }

    int phal_semaphore_broadcast(phal_semaphore *sem) {
        return ENOSYS;
    }

    static __attribute__((unused)) int timespec_subtract (struct timespec *result, const struct timespec *x, struct timespec *y) {
        /* Perform the carry for the later subtraction by updating y. */
        if (x->tv_nsec < y->tv_nsec) {
            int sec = (y->tv_nsec - x->tv_nsec) / 1000 * 1000 * 1000 + 1;
            y->tv_nsec -= 1000 * 1000 * 1000 * sec;
            y->tv_sec += sec;
        }
        if (x->tv_nsec - y->tv_nsec > 1000 * 1000 * 1000) {
            int sec = (y->tv_nsec - x->tv_nsec) / 1000 * 1000 * 1000;
            y->tv_nsec += 1000 * 1000 * 1000 * sec;
            y->tv_sec -= sec;
        }

        /* Compute the time remaining to wait.
        tv_usec is certainly positive. */
        result->tv_sec = x->tv_sec - y->tv_sec;
        result->tv_nsec = x->tv_nsec - y->tv_nsec;

        /* Return 1 if result is negative. */
        return x->tv_sec < y->tv_sec;
    }

    // TODO: This kinda sucks. Different platform behave differently here. For now
    // let's focus on the switch, which needs a locked mutex and a semaphore.
    /// Wait for the semaphore to be signaled. Note that we should **not** wake up
    /// if a signal was previously sent. This is not a counting semaphore.
    int phal_semaphore_wait(phal_semaphore *sem, const struct timespec *abstime)
    {
        return ENOSYS;
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
        return ENOSYS;
    }

    int phal_semaphore_unlock(phal_semaphore *sem)
    {
        return ENOSYS;
    }

    void **phal_get_tls()
    {
        return NULL; // F I X
    }


    int _fstat_r(struct _reent *reent, int file, struct stat *st)
    {
        reent->_errno = ENOSYS;
        return -1;
    }

    ssize_t _read_r(struct _reent *reent, int file, void *ptr, size_t len)
    {
        return -ENOSYS;
    }

    int _close_r(struct _reent *reent, int file)
    {
        return -1;
    }

    int _isatty_r(struct _reent *reent, int file)
    {
        reent->_errno = ENOSYS;
        return -1;
    }

    off_t _lseek_r(struct _reent *reent, int file, off_t pos, int whence)
    {
        return -ENOSYS;
    }
}