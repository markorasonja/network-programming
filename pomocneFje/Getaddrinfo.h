#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <err.h>

// Error-checking wrapper for getaddrinfo()
void Getaddrinfo(const char *node, const char *service, const struct addrinfo *hints, struct addrinfo **res) {
    int status = getaddrinfo(node, service, hints, res);
    if (status != 0) {
        errx(1, "Getaddrinfo failed: %s", gai_strerror(status));
    }
}