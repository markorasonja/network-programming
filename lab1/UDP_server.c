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
#include <unistd.h>
#define MAXLEN 512

int main(int argc, char *argv[]) {
    int ch;
    char *payload = NULL;
    char *PORT = NULL;
    while((ch = getopt(argc, argv, "l:p:")) != -1) {
        switch(ch){
            case 'l': 
                PORT = optarg;
                break;
            case 'p': 
                payload = optarg;
                break;
            case '?':
                errx(1, "Usage: ./UDP_server [-l port] [-p payload]\n");
        }
    }

    if(payload == NULL) {
        payload = "";
    }

    if(PORT == NULL){
        PORT = "1234";
    }

    int mysock;
    struct sockaddr cli;
    struct addrinfo hints, *res;

    char buf[MAXLEN];
    socklen_t clilen;
    int msglen;
    int gotlen;

    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    if(getaddrinfo(NULL, PORT, &hints, &res) == -1){
        errx(1, "problem do getaddrinfo");
    }

    mysock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    bind(mysock, res->ai_addr, res->ai_addrlen);

    while(1) {
        clilen = sizeof(cli);
        msglen = recvfrom(mysock, buf, MAXLEN, 0, &cli, &clilen);
        if(msglen == -1) {
            errx(1, "neuspjesno slusanje na poruke");
        }

        buf[5] = '\0';
        if(strcmp(buf,"HELLO") == 0) {
            printf("Uspjesno primljeno\n");
            printf("Saljem: %s\n", payload);
            gotlen = sendto(mysock, payload, strlen(payload), 0, &cli, clilen);
            if(gotlen == -1) {
                errx(1, "neuspjesno dobavljanje poruke");
            }
        }
        else {
            printf("Netocna poruka: %s\n", buf);
        }
    }

    return 0;
}