#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <err.h>

// Error-checking wrapper for socket()
int Socket(int family, int type, int protocol) {
    int sockfd = socket(family, type, protocol);
    if (sockfd == -1) {
        errx(1, "Socket creation failed: %s", strerror(errno));
    }
    return sockfd;
}