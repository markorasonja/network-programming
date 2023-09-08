#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <err.h>

// Error-checking wrapper for send()
ssize_t Send(int sockfd, const void *buf, size_t len, int flags) {
    ssize_t bytes_sent = send(sockfd, buf, len, flags);
    if (bytes_sent == -1) {
        errx(1, "Send failed: %s", strerror(errno));
    }
    return bytes_sent;
}
