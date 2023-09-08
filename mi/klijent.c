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

    char *PORT = "1234";
    char *IP = "127.0.0.1";
    int ch;
    while((ch = getopt(argc, argv, "p:s:")) != -1) {
        switch(ch){
            case 'p': 
                PORT = optarg;
                break;
            case 's': 
                IP = optarg;
                break;
            case '?':
                errx(1, "Usage: ./klijent [-p port] [-s host]\n");
        }
    }

    if(argc > 5) {
        errx(1, "Usage: ./klijent [-p port] [-s host]\n");
    }


    struct addrinfo hints, *res;

    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    Getaddrinfo(IP, PORT, &hints, &res);

    int mysock = Socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    char inputBuff[256];
    char msgToServ[261];
    char *randomChar = "AB";

    char *a = "blabla.hr";

    typedef struct {
        char *poc;
        char *id;
        char *msg;
        char *zad;
    } MSG;


    while(fgets(inputBuff, sizeof(inputBuff), stdin)) {

        struct sockaddr_in from;
        socklen_t fromaddrlen;

        MSG *m = malloc(sizeof(MSG));  
        m->poc = "\0\0";
        m->id = randomChar;
        m->msg = inputBuff;
        m->zad = "\0";

        int lenInput = strlen(inputBuff);
        
        strcat(msgToServ, m->poc);
        strcat(msgToServ, m->id);
        strcat(msgToServ, m->msg);
        strcat(msgToServ, m->zad);



        ssize_t sentToUdp = sendto(mysock, msgToServ, strlen(msgToServ),  0, res->ai_addr, res->ai_addrlen);
        if(sentToUdp == -1) {
            errx(1, "Neuspjesno slanje.");
        }

        char rec[256];

        size_t recvUdp = recvfrom(mysock, rec, 256, 0, (struct sockaddr *)&from, &fromaddrlen);
        if(recvUdp == -1) {
            errx(1, "Neuspjesno primanje.");
        }
        printf("%s", rec);

        free(m);
    }

    return 0;

}