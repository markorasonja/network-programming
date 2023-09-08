#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <err.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "Socket.h"

#define BACKLOG 10
#define MAX_L 1024

int main(int argc, char *argv[])
{

    char *PORT = "1234";

    FILE *fp = NULL;

    int ch;
    extern int optind;

    while((ch = getopt(argc, argv, "p:")) != -1) {
        switch(ch){
            case 'p': 
                PORT = optarg;
                break;
            case '?':
                errx(1, "Usage: ./tcpserver [-p port]\n");
        }
    }

    if(argc > 3) {
        errx(1, "Usage: ./tcpserver [-p port]\n");
    }

    int sockfd;
    struct sockaddr_in my_addr;
    struct sockaddr_in their_addr;
    socklen_t sin_size;
    char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
    int error;
    char msgbuf[MAX_L];

    typedef struct {
        int offset;
        char filename[MAX_L - 4];
    } MSG;

    typedef struct {
        char status[1];
        char poruka[MAX_L - 1];
    } MSGtoCL;

    sockfd = Socket(PF_INET, SOCK_STREAM, 0);

    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(atoi(PORT));
    my_addr.sin_addr.s_addr = INADDR_ANY;

    memset(my_addr.sin_zero, '\0', sizeof my_addr.sin_zero);

    int on=1;
    // "address already in use"; ako je TIME_WAIT sad ce proci!
    if (setsockopt(sockfd,
    SOL_SOCKET, SO_REUSEADDR,
    &on, sizeof(int)) == -1) {
        err(1,"setsockopt");
    }

    if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof my_addr) == -1) {
		err(1,"bind");
    }

    if (listen(sockfd, BACKLOG) == -1) {
		err(1,"listen");
    }

    for(;;) {
        int newfd;

        sin_size = sizeof their_addr;
        if ((newfd=accept(sockfd,(struct sockaddr *)&their_addr,&sin_size)) == -1) {
            err(1,"accept");
        }

        if ( (error = getnameinfo((struct sockaddr *)&their_addr, sin_size,
                                hbuf, sizeof(hbuf), sbuf, sizeof(sbuf),
                                0))) {
            errx(1, "%s", gai_strerror(error));
        }

        int total_bytes = 0;
        int got;

        while(total_bytes <= 4) {
            got = recv(newfd, msgbuf + total_bytes, sizeof(msgbuf) - total_bytes, 0);
            if(got == -1) {
                errx(1, "Bad receiving.");
            }
            total_bytes += got;
        }

        MSG *msg = (MSG *)msgbuf;

        MSGtoCL *msgtocl = malloc(sizeof(MSGtoCL));

        msg->filename[total_bytes-4] = '\0';
        msg->offset = ntohl(msg->offset);

        int exists = access(msg->filename, F_OK);
        if(exists != 0) {
            char *m = "Ne postoji trazena datoteka.";
            memcpy(msgtocl->status, "1", 1);
            memcpy(msgtocl->poruka, m, strlen(m));
            msgtocl->poruka[strlen(m)] = '\0';
            if (send(newfd, msgtocl, sizeof(char)+strlen(msgtocl->poruka), 0) == -1) {
                perror("send");
            }
            close(newfd);
        }

        else if(access(msg->filename, R_OK) != 0) {
            char *m = "Server nema prava na citanje.";
            memcpy(msgtocl->status, "2", 1);
            memcpy(msgtocl->poruka, m, strlen(m));
            msgtocl->poruka[strlen(m)] = '\0';
            if (send(newfd, msgtocl, sizeof(char)+strlen(msgtocl->poruka), 0) == -1) {
                perror("send");
            }
            close(newfd);
        }

        else {

            fp = fopen(msg->filename, "rb");
            fseek(fp, 0L, SEEK_END);
            long offset = ftell(fp);

            if (offset == (long)msg->offset) {
                char *m = "Datoteka vec u cjelosti postoji kod klijenta.";
                memcpy(msgtocl->status, "3", 1);
                memcpy(msgtocl->poruka, m, strlen(m));
                msgtocl->poruka[strlen(m)] = '\0';
                if (send(newfd, msgtocl, sizeof(char)+strlen(msgtocl->poruka), 0) == -1) {
                    perror("send");
                }
                close(newfd);
                continue;
            }
            else {
                fseek(fp, msg->offset, SEEK_SET);
            }

            printf("pocinjem s offseta: %d\n", msg->offset);
            
            char msgToCl[MAX_L];
            size_t nread;

            if (send(newfd, "0", 1, 0) == -1) {
                perror("send");
            }            

            while((nread = fread(msgToCl, 1, MAX_L, fp)) > 0) {
                if (send(newfd, msgToCl, nread, 0) == -1) {
                    perror("send");
                }
            }
            fclose(fp);
            printf("Poslan file: %s\n", msg->filename);
            close(newfd);
        }
        free(msgtocl);
    }
    close(sockfd);

    return 0;
}