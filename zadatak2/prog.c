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

int TCP_FLAG = 0; //default
int REVERSE_LOOKUP = 0;
int UDP_FLAG = 0;
int HEX = 0;
int HOST_BYTE_ORDER = 0; //default
int NETWORK_BYTE_ORDER = 0;
int IPv4 = 0; //default
int IPv6 = 0;


int main(int argc, char *argv[])
{
    int ch;
    int i;
    extern int optind;

    while((ch = getopt(argc, argv, "rtuxhn46")) != -1) {
        switch (ch)
        {
        case 'r':
            REVERSE_LOOKUP = 1;
            break;

        case 't':
            TCP_FLAG = 1;
            break;

        case 'u':
            UDP_FLAG = 1;
            break;

        case 'x':
            HEX = 1;
            break;

        case 'h':
            HOST_BYTE_ORDER = 1;
            break;

        case 'n':
            NETWORK_BYTE_ORDER = 1;
            break;

        case '4':
            IPv4 = 1;
            break;

        case '6':
            IPv6 = 1;
            break;

        default:
            break;
        }
    }

    if(argv[optind] == NULL || argv[optind + 1] == NULL) {
        errx(2, "prog [-r] [-t|-u] [-x] [-h|-n] [-46] {hostname|IP_address} {servicename|port}");
    }
    
    if(REVERSE_LOOKUP) {

        char *IPADDR = argv[optind];

        int PORT = atoi(argv[optind + 1]);
        char HOSTNAME[100];
        
        char SERVICE_NAME[100];
        
        struct sockaddr_in sa;
        sa.sin_port = htons(PORT);
        sa.sin_family = AF_INET;

        struct sockaddr_in6 sa6;
        sa6.sin6_port = htons(PORT);
        sa6.sin6_family = AF_INET6;

        if(!IPv6 || IPv4) {
            if(inet_pton(AF_INET, IPADDR, &(sa.sin_addr)) != 1) {
                errx(1, "%s nije valjanja adresa", IPADDR);
            }

            if(!UDP_FLAG || TCP_FLAG) {
                int e = getnameinfo((struct sockaddr *)&sa,
                                sizeof(struct sockaddr_in),
                                HOSTNAME, sizeof(HOSTNAME),
                                SERVICE_NAME, sizeof(SERVICE_NAME), 
                                0);
            
                if(e) errx(2, "getnameinfo: %s", gai_strerror(e));

                printf("%s (%s) %s\n", IPADDR, HOSTNAME, SERVICE_NAME);

            }
            else {
                int e = getnameinfo((struct sockaddr *)&sa,
                                sizeof(struct sockaddr_in),
                                HOSTNAME, sizeof(HOSTNAME),
                                SERVICE_NAME, sizeof(SERVICE_NAME), 
                                NI_DGRAM);
            
                if(e) errx(2, "getnameinfo: %s", gai_strerror(e));

                printf("%s (%s) %s\n", IPADDR, HOSTNAME, SERVICE_NAME);

            }
        }
        else {
            if(inet_pton(AF_INET6, IPADDR, &(sa6.sin6_addr)) != 1) {
                errx(1, "%s nije valjanja adresa", IPADDR);
            }
            if(!UDP_FLAG || TCP_FLAG) {
                int e = getnameinfo((struct sockaddr *)&sa6,
                                sizeof(struct sockaddr_in6),
                                HOSTNAME, sizeof(HOSTNAME),
                                SERVICE_NAME, sizeof(SERVICE_NAME), 
                                0);
            
                if(e) errx(2, "getnameinfo: %s", gai_strerror(e));

                printf("%s (%s) %s\n", IPADDR, HOSTNAME, SERVICE_NAME);

            }
            else {
                int e = getnameinfo((struct sockaddr *)&sa6,
                                sizeof(struct sockaddr_in6),
                                HOSTNAME, sizeof(HOSTNAME),
                                SERVICE_NAME, sizeof(SERVICE_NAME), 
                                NI_DGRAM);
            
                if(e) errx(2, "getnameinfo: %s", gai_strerror(e));

                printf("%s (%s) %s\n", IPADDR, HOSTNAME, SERVICE_NAME);
            };
        }



    }
    
    else {

        int PORT;
        char *HOSTNAME = argv[optind];
        char *IPADDR;
        char *SERVICE_NAME = argv[optind + 1];
        

        struct addrinfo hints, *res;

        char addrstr[100];

        if(!IPv6 || IPv4) {
            memset(&hints, 0, sizeof(hints));
            hints.ai_family = AF_INET;
            hints.ai_flags |= AI_CANONNAME;

            if(!UDP_FLAG || TCP_FLAG) {
                hints.ai_socktype = SOCK_STREAM;
                int error = getaddrinfo(HOSTNAME, SERVICE_NAME, &hints, &res);

                if (error) errx(1, "%s", gai_strerror(error));

                while(res) {
                    inet_ntop(
                        res->ai_family,
                        &((struct sockaddr_in *) res->ai_addr)->sin_addr,
                        addrstr, 100);

                    if(!HEX) {
                        if(!NETWORK_BYTE_ORDER || HOST_BYTE_ORDER) {
                            printf("%s (%s) %d\n", addrstr, res->ai_canonname, ntohs(((struct sockaddr_in *) res->ai_addr)->sin_port));
                        }
                        else {
                            printf("%s (%s) %d\n", addrstr, res->ai_canonname, (((struct sockaddr_in *) res->ai_addr)->sin_port));
                        }
                    }
                    else {
                        if(!NETWORK_BYTE_ORDER || HOST_BYTE_ORDER) {
                            printf("%s (%s) %04X\n", addrstr, res->ai_canonname, ntohs(((struct sockaddr_in *) res->ai_addr)->sin_port));
                        }
                        else {
                            printf("%s (%s) %04X\n", addrstr, res->ai_canonname, (((struct sockaddr_in *) res->ai_addr)->sin_port));
                        }
                    }
                    break;
                }
            }
            else {
                hints.ai_socktype = SOCK_DGRAM;
                int error = getaddrinfo(HOSTNAME, SERVICE_NAME, &hints, &res);

                if (error) errx(1, "%s", gai_strerror(error));

                while(res) {
                    inet_ntop(
                        res->ai_family,
                        &((struct sockaddr_in *) res->ai_addr)->sin_addr,
                        addrstr, 100);
                    if(!HEX) {
                        if(!NETWORK_BYTE_ORDER || HOST_BYTE_ORDER) {
                            printf("%s (%s) %d\n", addrstr, res->ai_canonname, ntohs(((struct sockaddr_in *) res->ai_addr)->sin_port));
                        }
                        else {
                            printf("%s (%s) %d\n", addrstr, res->ai_canonname, (((struct sockaddr_in *) res->ai_addr)->sin_port));
                        }
                    }
                    else {
                        if(!NETWORK_BYTE_ORDER || HOST_BYTE_ORDER) {
                            printf("%s (%s) %04X\n", addrstr, res->ai_canonname, ntohs(((struct sockaddr_in *) res->ai_addr)->sin_port));
                        }
                        else {
                            printf("%s (%s) %04X\n", addrstr, res->ai_canonname, (((struct sockaddr_in *) res->ai_addr)->sin_port));
                        }
                    }
                    break;
                }
            }
        }
        else {
            memset(&hints, 0, sizeof(hints));
            hints.ai_family = AF_INET6;
            hints.ai_flags |= AI_CANONNAME;

            if(!UDP_FLAG || TCP_FLAG) {

                hints.ai_socktype = SOCK_STREAM;
                int error = getaddrinfo(HOSTNAME, SERVICE_NAME, &hints, &res);

                if (error) errx(1, "%s", gai_strerror(error));

                while(res) {
                    inet_ntop(
                        res->ai_family,
                        &((struct sockaddr_in6 *) res->ai_addr)->sin6_addr,
                        addrstr, 100);
                    if(!HEX) {
                        if(!NETWORK_BYTE_ORDER || HOST_BYTE_ORDER) {
                            printf("%s (%s) %d\n", addrstr, res->ai_canonname, ntohs(((struct sockaddr_in *) res->ai_addr)->sin_port));
                        }
                        else {
                            printf("%s (%s) %d\n", addrstr, res->ai_canonname, (((struct sockaddr_in *) res->ai_addr)->sin_port));
                        }
                    }
                    else {
                        if(!NETWORK_BYTE_ORDER || HOST_BYTE_ORDER) {
                            printf("%s (%s) %04X\n", addrstr, res->ai_canonname, ntohs(((struct sockaddr_in *) res->ai_addr)->sin_port));
                        }
                        else {
                            printf("%s (%s) %04X\n", addrstr, res->ai_canonname, (((struct sockaddr_in *) res->ai_addr)->sin_port));
                        }
                    }
                    break;
                }
            }
            else {
                hints.ai_socktype = SOCK_DGRAM;
                int error = getaddrinfo(HOSTNAME, SERVICE_NAME, &hints, &res);

                if (error) errx(1, "%s", gai_strerror(error));

                while(res) {
                    inet_ntop(
                        res->ai_family,
                        &((struct sockaddr_in6 *) res->ai_addr)->sin6_addr,
                        addrstr, 100);
                    if(!HEX) {
                        if(!NETWORK_BYTE_ORDER || HOST_BYTE_ORDER) {
                            printf("%s (%s) %d\n", addrstr, res->ai_canonname, ntohs(((struct sockaddr_in *) res->ai_addr)->sin_port));
                        }
                        else {
                            printf("%s (%s) %d\n", addrstr, res->ai_canonname, (((struct sockaddr_in *) res->ai_addr)->sin_port));
                        }
                    }
                    else {
                        if(!NETWORK_BYTE_ORDER || HOST_BYTE_ORDER) {
                            printf("%s (%s) %04X\n", addrstr, res->ai_canonname, ntohs(((struct sockaddr_in *) res->ai_addr)->sin_port));
                        }
                        else {
                            printf("%s (%s) %04X\n", addrstr, res->ai_canonname, (((struct sockaddr_in *) res->ai_addr)->sin_port));
                        }
                    }
                    break;
                }
            }
        }
    }

    return 0;
}


