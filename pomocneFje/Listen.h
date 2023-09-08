#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <err.h>

// Error-checking wrapper for listen()
void Listen(int sockfd, int backlog) {
    if (listen(sockfd, backlog) == -1) {
        errx(1, "Listen failed: %s", strerror(errno));
    }
}