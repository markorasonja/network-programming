#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <err.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <stdbool.h>
#include <time.h>

#define MAX_L 1024

int
main(int argc, char *argv[])
{

    char *PORT = "1234";
    char *SERVER = "127.0.0.1";
    bool c = false;
    char stream[MAX_L];

    FILE *fp = NULL;

    int ch;
    extern int optind;

    typedef struct {
        int32_t offset;
        char filename[MAX_L - 4];
    } MSG;
 
    if(argc <= 1) {
        errx(3, "Usage: ./tcpklijent [-s server] [-p port] [-c] filename\n");
    }

    while((ch = getopt(argc, argv, "s:p:c")) != -1) {
        switch(ch){
            case 'p': 
                PORT = optarg;
                break;
            case 's': 
                SERVER = optarg;
                break;
            case 'c': 
                c = true;
                break;
            case '?':
                errx(3, "Usage: ./tcpserver [-p port]\n");
        }
    }

    if(argc > 7) {
        errx(3, "Usage: ./tcpserver [-p port]\n");
    }

    char *file = argv[argc - 1];
    int sockfd, numbytes;
    struct sockaddr_in their_addr;
    int error;
    struct addrinfo hints, *res;

    //provjera dal c nije tu a file postoji
    if(!c) {
        if(access(file, F_OK) == 0){
            errx(1, "File exists and continue is not set up.");
        }
    };

    MSG *msgToServer = malloc(sizeof(MSG));

    long offset = 0L;

    //ako c, a file ne postoji
    if(access(file, F_OK) != 0) {
        offset = 0L;
    }
    //file postoji
    else {
        int isWritable = access(file, W_OK);
        if(isWritable != 0) {
            errx(2, "Cannot write to file");
        }
        if(c) {
            fp = fopen(file, "r");
            fseek(fp, 0L, SEEK_END);
            offset = ftell(fp);
            fclose(fp);
        }
    } 

    int32_t o = htonl(offset);
    
    msgToServer->offset = o;
    memcpy(msgToServer->filename, file, sizeof(msgToServer->filename));
    msgToServer->filename[strlen(file)] = '\0';

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;

    
    if ((error=getaddrinfo(SERVER, PORT, &hints, &res)))
        errx(1, "%s", gai_strerror(error));

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
	errx(1,"socket");
    }

    their_addr.sin_family = AF_INET;
    their_addr.sin_port = htons(atoi(PORT));

    their_addr.sin_addr = ((struct sockaddr_in *)res->ai_addr)->sin_addr;
    memset(their_addr.sin_zero, '\0', sizeof their_addr.sin_zero);

    if (connect(sockfd,(struct sockaddr *)&their_addr,sizeof their_addr) == -1) {
	    err(1,"connect did not succed.");
    }
    
    if ((numbytes = send(sockfd, msgToServer, strlen(msgToServer->filename) + 4, 0)) == -1) {
	    errx(1,"send dont work");
    }

    char status[2];

    if(recv(sockfd, status, 1, 0) == -1) {
        errx(1,"recv dont work");
    }

    status[1] = '\0';
    int got;

    char e[MAX_L];
    
    if((strcmp(status, "1")) == 0){
        if((got = recv(sockfd, e, MAX_L, 0)) == -1) {
            errx(1,"recv dont work");
        }
        e[got] = '\0';
        close(sockfd);
        free(msgToServer);
        errx(1, "Poruka s servera -> %s\n", e);
    }

    if((strcmp(status, "2")) == 0) {
        if((got = recv(sockfd, e, MAX_L, 0)) == -1) {
            errx(1,"recv dont work");
        }
        e[got] = '\0';
        close(sockfd);
        free(msgToServer);
        errx(2, "Poruka s servera -> %s\n", e);
    }

    if((strcmp(status, "3")) == 0) {
        if((got = recv(sockfd, e, MAX_L, 0)) == -1) {
            errx(1,"recv dont work");
        }
        e[got] = '\0';
        close(sockfd);
        free(msgToServer);
        errx(3, "Poruka s servera -> %s\n", e);
    }

    fp = fopen(file, "a");

    fseek(fp, 0L, SEEK_END);

    size_t nread;

    while((nread = recv(sockfd, stream, MAX_L, 0)) > 0) {
        fwrite(stream, 1, nread, fp);
    }
    
    // if ((numbytes = recv(sockfd, msgFromServer, strlen(msgFromServer), 0)) == -1) {
	//     err(1,"recv");
    // }
    fclose(fp);
    close(sockfd);
    free(msgToServer);

    return 0;
}
