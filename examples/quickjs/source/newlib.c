#include <time.h>
#include <sys/types.h>
#include <sys/reent.h>
#include <errno.h>
#include <sys/select.h>

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

int _rename_r(struct _reent *reent, const char *old, const char *new)
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