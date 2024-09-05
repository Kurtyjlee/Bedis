#include <errno.h>
#include <iostream>
#include <unistd.h>
#include <cassert>
#include <fcntl.h>
#include <vector>

#include "constants.h"

/**
 * Kill the process with an error message
 */
void die(const char *msg)
{
    int err = errno;
    fprintf(stderr, "[%d] %s\n", err, msg);
    abort();
}

/**
 * Reads n bytes from fd to buf
 */
int32_t read_full(int fd, char *buf, size_t n)
{
    while (n > 0)
    {
        ssize_t rv = read(fd, buf, n);
        if (rv <= 0)
            return -1;

        assert((size_t)rv <= n);
        n -= (size_t)rv;
        buf += rv;
    }
    return 0;
}

/**
 * Write n bytes to fd
 */
int32_t write_all(int fd, const char *buf, size_t n)
{
    while (n > 0)
    {
        ssize_t rv = write(fd, buf, n);
        if (rv <= 0)
            return -1;
        assert((size_t)rv <= n);
        n -= (size_t)rv;
        buf += rv;
    }
    return 0;
}

/**
 * Set this fd to nonblocking mode for our event loops
 */
void fd_set_nb(int fd)
{
    errno = 0;

    // Retrieve the file status flags
    int flags = fcntl(fd, F_GETFL, 0);
    if (errno)
    {
        die("fcntl error");
        return;
    }

    flags |= O_NONBLOCK;

    errno = 0;
    (void)fcntl(fd, F_SETFL, flags);
    if (errno)
        die("fcntl error");
}

void msg(const char *msg) {
    std::cerr << msg << std::endl;
}
