#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <err.h>

// Error-checking wrapper for accept()
int Accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
    int connfd = accept(sockfd, addr, addrlen);
    if (connfd == -1) {
        errx(1, "Accept failed: %s", strerror(errno));
    }
    return connfd;
}