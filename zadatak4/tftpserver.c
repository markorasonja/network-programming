
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <err.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "Bind.h"
#include "Error_handle.h"
#include "Getaddrinfo.h"
#include "Socket.h"
#include "Read_netascii.h"
#include "Recvfrom.h"
#include "Sendto.h"
#include "Deamon_init.h"
#define LOG_name "mr53609:MrePro tftpserver"


int main(int argc, char *argv[])
{
    int dflag = 0;
    int ch;
    extern char *optarg;
    int brojnovih = 0;

    //int port = 69;
    int pid;
    int sockfd;
    struct sockaddr_in cliaddr;
    socklen_t len;
    struct addrinfo hints, *res;


    while((ch = getopt(argc, argv, "d")) != -1)
        switch(ch) {
            case 'd':
                dflag = 1;
                break;
            case '?':
            default:
                errx(1,"Usage: ./tftpserver [-d] port\n");
        }

    if (argc!=2 && argc!=3){
        errx(1,"Usage: ./tftpserver [-d] port\n");
	}
	if (strstr(argv[argc-1],"-d")){
		errx(1,"Usage: ./tftpserver [-d] port\n");
	}
	if (strstr(argv[argc-1],"tftpserver")){
		errx(1,"Usage: ./tftpserver [-d] port\n");
	}
	if (argc == 1){
		errx(1,"Usage: ./tftpserver [-d] port\n");
	}
    if (argc > 3 || (argc == 3 && dflag == 0)){
        errx(1,"Usage: ./tftpserver [-d] port\n");
    }

    char *port_string = (char *)malloc(10);
    port_string = argv[argc-1];



    //socket things
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; // IPv4
    hints.ai_socktype = SOCK_DGRAM; // UDP
    hints.ai_flags = AI_PASSIVE; // use my IP

    int e = getaddrinfo(NULL, port_string, &hints, &res);
    if (e != 0) {
        errx(1,"getaddrinfo: %s\n", gai_strerror(e));
    }

    //deamon
    if (dflag == 1){
        Deamon_init(LOG_name, LOG_DAEMON);
    }

    sockfd = Socket(res->ai_family, res->ai_socktype, res->ai_protocol, dflag);
    Bind(sockfd, res->ai_addr, res->ai_addrlen, dflag);
    freeaddrinfo(res);

    //main loop
    for (;;)
    {
        char buff[516];
        memset(&cliaddr, 0, sizeof(cliaddr));
        len = sizeof(cliaddr);
        Recvfrom(sockfd, buff, sizeof(buff), 0, (struct sockaddr *) &cliaddr, &len, dflag);

        if((pid = fork()) == 0)
        {
            //child
            FILE *fp;
            int newsockfd;
            int zastavica_za_novi_red = 0;
            int netascii_flag = 0;
            int flag = 0;

            unsigned short opcode = (buff[0] << 8) | buff[1];  // Extract opcode
            char filename[512];
            strncpy(filename, buff + 2, 512 - 1);  // Extract filename
            filename[512 - 1] = '\0';  // Ensure null-terminated string

            char mode[100];
            strncpy(mode, buff + 2 + strlen(filename) + 1, 100 - 1);  // Extract mode
            mode[100 - 1] = '\0';  // Ensure n`ull-terminated string

            
            if(opcode != 1){
                printf("opcode != 1\n");
                char *message_reply = (char *)malloc(4);
                memset(message_reply, 0, 4);
                uint16_t opcode_reply = 5;
                uint16_t error_code = 4;
                opcode_reply = htons(opcode_reply);
                error_code = htons(4);
                memcpy(message_reply, &opcode_reply, 2);
                memcpy(message_reply + 2, &error_code, 2);
                char *error_message = "Illegal TFTP operation.";
                memcpy(message_reply + 4, error_message, strlen(error_message));
                Sendto(sockfd, message_reply, strlen(error_message) + 4, 0, (struct sockaddr *) &cliaddr, len, dflag);
                if (dflag == 1){
                    syslog(LOG_ALERT, "TFTP ERROR Illegal TFTP operation.\n");
                }
                else
                {
                    errx(1, "TFTP ERROR Illegal TFTP operation.\n");
                }
            }

            // if ( strcmp(mode, "octet") == 0 ) {
            //     int octet_flag = 1;
            // }
            if ( strcmp(mode, "netascii") == 0 ) netascii_flag = 1;

            char adr[255];
            inet_ntop(AF_INET, &(cliaddr.sin_addr), adr, INET_ADDRSTRLEN);
            if (dflag == 1){
                syslog(LOG_INFO, "%s->%s\n", adr, filename);
            }
            else
            {
                printf("%s->%s\n", adr, filename);
            }

            // //zadano /tftpboot/filename
            // if (strchr(filename, '/')) {
            //     filename = filename + 10;
            // }

            int error = chdir("/tftpboot");
            if(error == -1)
            {
                errx(1, "Error: chdir");
            }
        
            hints.ai_family = AF_INET; // IPv4
            hints.ai_socktype = SOCK_DGRAM; // UDP
            hints.ai_flags = AI_PASSIVE; // use my IP

            Getaddrinfo(NULL, "0", &hints, &res, dflag);

            newsockfd = Socket(res->ai_family, res->ai_socktype, res->ai_protocol, dflag);

            if (netascii_flag == 0) {
                fp = fopen(filename, "rb");
            }
            else {
                fp = fopen(filename, "r");
            }

            if (fp == NULL)
            {
                printf("File not found.\n");
                char *message_reply = (char *)malloc(255);
                memset(message_reply, 0, 4);
                uint16_t opcode_reply = 5;
                uint16_t error_code = 1;
                opcode_reply = htons(opcode_reply);
                error_code = htons(1);
                memcpy(message_reply, &opcode_reply, 2);
                memcpy(message_reply + 2, &error_code, 2);
                char *error_message = "File not found.";
                memcpy(message_reply + 4, error_message, strlen(error_message));
                Sendto(sockfd, message_reply, strlen(error_message) + 4, 0, (struct sockaddr *) &cliaddr, len, dflag);
                free(message_reply);
                if (dflag == 1){
                    syslog(LOG_ALERT, "TFTP ERROR File not found.\n");
                }
                else
                {
                    fprintf(stderr, "TFTP ERROR File not found.\n");
                }
                continue;
            }

            // otvoren file
            char buffer[512];
            uint16_t pack_number = 0;
            
            int read_bytes = 0;
            char message_buffer[516];
            struct timeval tv;
            tv.tv_sec = 1;
            tv.tv_usec = 0;

            if (setsockopt(newsockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
                if (dflag == 1){
                    syslog(LOG_ALERT, "TFTP ERROR setsockopt.\n");
                }
                else
                {
                    fprintf(stderr, "TFTP ERROR setsockopt.\n");
                }
            }

            void *message_reply = (void *)malloc(516);

            while(1)
            {
                memset(buffer, 0, 512);
                memset(message_reply, 0, 516);
                if (netascii_flag)
                {
                    // printf("FLAG is: %d\n", flag);
                    read_bytes = Read_netascii(buffer, fp, &flag, &brojnovih, &zastavica_za_novi_red);
                }
                else
                {
                    read_bytes = fread(buffer, 1, 512, fp);
                }

                opcode = htons(3);
                pack_number++;
                int retransmit_number = 0;
                pack_number = htons(pack_number);
                memcpy(message_reply, &opcode, 2);
                memcpy(message_reply + 2, &pack_number, 2);
                memcpy(message_reply + 4, buffer, read_bytes);
                pack_number = ntohs(pack_number);


                if (netascii_flag && read_bytes < 512)
                {
                    read_bytes = read_bytes - 2;
                }

                while (retransmit_number < 3)
                {
                    Sendto(newsockfd, message_reply, 4 + read_bytes, 0, (struct sockaddr *) &cliaddr, len, dflag);
                    error = recvfrom(newsockfd, message_buffer, 516, 0, (struct sockaddr *) &cliaddr, &len);
                    if (error < 0)
                    {
                        retransmit_number++;

                        continue;
                    }
                    else
                    {
                        break;
                    }
                }
                

                
                if (retransmit_number > 3)
                {
                    if (dflag == 1){
                        syslog(LOG_ALERT, "TFTP ERROR retransmit.\n");
                        return -1;
                    }
                    else
                    {
                        errx(1, "TFTP ERROR retransmit.\n");
                    }
                    break;
                }



                uint16_t *opcodePtr;
                opcodePtr = (uint16_t *)message_buffer;
                *opcodePtr = ntohs(*opcodePtr);


                if (*opcodePtr != 4) {
                    memset(message_reply, 0, 516);
                    opcode = htons(5);
                    uint16_t error_code = 4;
                    error_code = htons(error_code);
                    memcpy(message_reply, &opcode, 2);
                    memcpy(message_reply + 2, &error_code, 2);
                    char *error_message = "No ACK received.";
                    memcpy(message_reply + 4, error_message, strlen(error_message));
                    Sendto(sockfd, message_reply, strlen(error_message) + 4, 0, (struct sockaddr *) &cliaddr, len, dflag);
                    if (dflag == 1){
                        syslog(LOG_ALERT, "TFTP ERROR No ACK received.\n");
                    }
                    else
                    {
                        errx(1, "TFTP ERROR No ACK received.\n");
                    }
                    continue;
                }

                // uint32_t *pack_numberPtr;
                // uint16_t pack_number_recv;
                // pack_numberPtr = (uint32_t *)(message_buffer + 2);
                // pack_number_recv = ntohs(*pack_numberPtr);
                
                // if (pack_number_recv == pack_number)
                // {
                //     memset(message_reply, 0, 516);
                //     opcode = htons(4);
                //     pack_number = htons(pack_number);
                //     memcpy(message_reply, &opcode, 2);
                //     memcpy(message_reply + 2, &pack_number, 2);
                //     char *message = "ACK";
                //     memcpy(message_reply + 4, message, strlen(message));
                //     Sendto(sockfd, message_reply, strlen(message) + 4, 0, (struct sockaddr *) &cliaddr, len, dflag);

                //     if (dflag == 1){
                //         syslog(LOG_INFO, "TFTP INFO retransmissions");
                //     }
                //     else
                //     {
                //         fprintf(stderr, "TFTP INFO retransmissions");
                //     }
                //     continue;
                // }

                // printf("pack_number: %d\n", pack_number);

                if (read_bytes < 512)
                {
                    exit(0);
                }
                // printf("pack_number: %d\n", pack_number);
                // printf("brojnovih: %d\n", brojnovih);
                // printf("pack_number*512 - brojnovih: %d\n", pack_number*512 - brojnovih);

                fseek(fp, (pack_number)*512 - brojnovih, SEEK_SET);
                zastavica_za_novi_red = 0;
                retransmit_number = 0;
            }
        }
    }
    return 0;
}