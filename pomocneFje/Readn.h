#include <errno.h>
#include <unistd.h>

ssize_t readn(int fd, void *buf, size_t n) {
    size_t nleft = n;
    char *bufptr = (char*)buf;

    while (nleft > 0) {
        ssize_t nread = read(fd, bufptr, nleft);

        if (nread == -1) {
            if (errno == EINTR) {
                continue; // interrupted by signal, try again
            }
            else {
                return -1; // error occurred
            }
        }
        else if (nread == 0) {
            break; // EOF reached
        }

        nleft -= nread;
        bufptr += nread;
    }

    return (n - nleft);
}