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
#include <sys/time.h>
#define MAXLEN 512

int main(int argc, char *argv[]) {
    int ch;
    char *payload = "\n";
    char *PORT_TCP = "1234";
    char *PORT_UDP = "1234";
    int opt_num = 0;
    while((ch = getopt(argc, argv, "t:u:p:")) != -1) {
        switch(ch){
            case 't': 
                PORT_TCP = optarg;
                opt_num++;
                break;
            case 'u':
                PORT_UDP = optarg;
                opt_num++;
                break;
            case 'p': 
                payload = optarg;
                payload = strcat(payload, "\n");
                opt_num++;
                break;
            case '?':
                errx(1, "Usage: ./server [-l port] [-p popis]\n");
        }
    }

    if(argc > (1 + 2*opt_num)) {
        errx(1, "Usage: ./server [-t tcp_port] [-u udp_port] [-p popis]\n");
    }

    int tcpsock;
    int udpsock;
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

    if(getaddrinfo(NULL, PORT_UDP, &hints, &res) == -1){
        errx(1, "problem do getaddrinfo");
    }

    udpsock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    setsockopt(udpsock, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int));
    bind(udpsock, res->ai_addr, res->ai_addrlen);
    
    freeaddrinfo(res);
    
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if(getaddrinfo(NULL, PORT_TCP, &hints, &res) == -1){
        errx(1, "problem do getaddrinfo");
    }

    tcpsock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    setsockopt(tcpsock, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int));
    bind(tcpsock, res->ai_addr, res->ai_addrlen);
    listen(tcpsock, 5);

    freeaddrinfo(res);

    int maxfd = tcpsock;
    if(udpsock > maxfd) {
        maxfd = udpsock;
    }

    fd_set master;
    fd_set read_fds;
    
    FD_ZERO(&read_fds);
    FD_ZERO(&master);
    FD_SET(0, &master);
    FD_SET(tcpsock, &master);
    FD_SET(udpsock, &master);
    
    for(;;) {
        read_fds = master;
        if(select(maxfd+1, &read_fds, NULL, NULL, NULL) == -1) {
            errx(1, "neuspjesno slusanje na poruke - select");
        }

        if(FD_ISSET(0, &read_fds)) {
            fgets(buf, MAXLEN, stdin);
            if(strcmp(buf, "QUIT\n") == 0) {
                printf("Izlazim\n");
                close(tcpsock);
                close(udpsock);
                exit(0);
            }
            else if(strcmp(buf, "PRINT\n") == 0) {
                printf("Payload: %s\n", payload);
            }
            else if(strncmp(buf, "SET", 3) == 0) {
                sscanf(buf, "SET %[^'\n']", payload);
                payload = strcat(payload, "\n");
                printf("Postavljam payload na: %s\n", payload);
            }
            else {
                printf("Nepoznata naredba\n");
            }
        }

        if(FD_ISSET(tcpsock, &read_fds)) {
            printf("TCP konekcija\n");
            int newfd;

            clilen = sizeof cli;
            if ((newfd=accept(tcpsock,(struct sockaddr *)&cli, &clilen)) == -1) {
                err(1,"accept");
            };

            msglen = recv(newfd, buf, MAXLEN, 0);
            if(msglen == -1) {
                errx(1, "neuspjesno slusanje na poruke");
            }
            buf[msglen] = '\0';

            printf("Primljeno: %s\n", buf);
            if(strcmp(buf,"HELLO") == 0) {
                printf("Uspjesno primljeno\n");
                printf("Saljem: %s\n", payload);
                gotlen = send(newfd, payload, strlen(payload), 0);
                if(gotlen == -1) {
                    errx(1, "neuspjesno dobavljanje poruke");
                }
                close(newfd);
            }
            else {
                printf("Netocna poruka: %s\n", buf);
                close(newfd);
            }
        }

        if(FD_ISSET(udpsock, &read_fds)){
            printf("UDP konekcija\n");
            clilen = sizeof(cli);
            msglen = recvfrom(udpsock, buf, MAXLEN, 0, &cli, &clilen);
            if(msglen == -1) {
                errx(1, "neuspjesno slusanje na poruke");
            }
            buf[msglen] = '\0';
            printf("Primljeno: %s\n", buf);
            if(strcmp(buf,"HELLO") == 0) {
                printf("Uspjesno primljeno\n");
                printf("Saljem: %s\n", payload);
                gotlen = sendto(udpsock, payload, strlen(payload), 0, &cli, clilen);
                if(gotlen == -1) {
                    errx(1, "neuspjesno dobavljanje poruke");
                }
            }
            else {
                printf("Netocna poruka: %s\n", buf);
            }
        }
    };

    return 0;
}