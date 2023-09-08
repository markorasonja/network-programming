#include <errno.h>
#include <unistd.h>

ssize_t writen(int fd, const void *buf, size_t n) {
    size_t nleft = n;
    const char *bufptr = (const char*)buf;

    while (nleft > 0) {
        ssize_t nwritten = write(fd, bufptr, nleft);

        if (nwritten == -1) {
            if (errno == EINTR) {
                continue; // interrupted by signal, try again
            }
            else {
                return -1; // error occurred
            }
        }

        nleft -= nwritten;
        bufptr += nwritten;
    }

    return n;
}