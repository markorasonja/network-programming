#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <err.h>
#include <errno.h>
#include <stdbool.h>
#include <time.h>
#include "Getaddrinfo.h"
#include "Socket.h"


int main(int argc, char *argv[]) {

    char *IP = "127.0.0.1";
    char *PORT = "1234";
    int ch;
    while((ch = getopt(argc, argv, "p:s:")) != -1) {
        switch(ch){
            case 'p': 
                PORT = optarg;
                break;
            case '?':
                errx(1, "Usage: ./server [-p naziv_ili_port]\n");
        }
    }

    if(argc > 3) {
        errx(1, "Usage: ./server [-p naziv_ili_port]\n");
    }

    int mysock;
    struct sockaddr cli;
    struct addrinfo hints, *res;

    char buf[261];
    socklen_t clilen;
    int msglen;
    int gotlen;

    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    Getaddrinfo(NULL, PORT, &hints, &res);

    mysock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    bind(mysock, res->ai_addr, res->ai_addrlen);

    freeaddrinfo(res);

    while(1) {
        clilen = sizeof(cli);


        msglen = recvfrom(mysock, buf, 261, 0, &cli, &clilen);
        if(msglen == -1) {
            errx(1, "neuspjesno slusanje na poruke");
        }

        char ip[256];
        strncpy(ip, buf + 2, msglen - 5);

        printf("%s",ip);



        char *random;
        strncpy(rand, buf, 2);

        // char name[msglen-5];
        // memcpy(name, buf + 4, msglen - 1);

        struct addrinfo *res;
        getaddrinfo(ip, NULL, &hints, &res);

        char sendback[256];
        int num = 0;

        while(res != NULL) {
            strcat(sendback, res->ai_addr->sa_data);
            res = res->ai_next;
            num += 1;
        }

        char msgToCl[256];

        char *status = "0";

        if(num = 0) {
            status = "1";
        }

        memcpy(msgToCl, status, 1);
        memcpy(msgToCl + 1, random, 2);
        memcpy(msgToCl + 3, sendback, strlen(sendback));



        gotlen = sendto(mysock, msgToCl, strlen(msgToCl), 0, &cli, clilen);
            if(gotlen == -1) {
                errx(1, "neuspjesno dobavljanje poruke");
            }

    }


    return 0;
}