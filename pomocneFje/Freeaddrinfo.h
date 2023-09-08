#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <err.h>

// Error-checking wrapper for freeaddrinfo()
void Freeaddrinfo(struct addrinfo *res) {
    freeaddrinfo(res);
}