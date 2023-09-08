#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <err.h>

// Error-checking wrapper for recv()
ssize_t Recv(int sockfd, void *buf, size_t len, int flags) {
    ssize_t bytes_received = recv(sockfd, buf, len, flags);
    if (bytes_received == -1) {
        errx(1, "Recv failed: %s", strerror(errno));
    }
    return bytes_received;
}