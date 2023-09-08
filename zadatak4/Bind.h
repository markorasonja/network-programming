#include <syslog.h>
#include <errno.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>


int Bind (int sockfd, const struct sockaddr *myaddr, int addrlen, int dflag){
	int erro;
	erro=bind(sockfd, myaddr, addrlen);
	if (erro<0){
		if (dflag){
			syslog(LOG_ALERT, "TFTP ERROR %d bind\n", errno);
			return -1;
		}else{
			errx(1,"TFTP ERROR %d bind\n",errno);
		}
	}else{
		return 0;
	}
}