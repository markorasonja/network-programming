#include <syslog.h>
#include <errno.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>


ssize_t Recvfrom(int sockfd, void *buff, size_t nbytes, int flags, struct sockaddr* from, socklen_t *fromaddrlen, int dflag){
	int n;
	n=recvfrom(sockfd, buff, nbytes, flags, from, fromaddrlen);
	if (n<0){
		if (dflag){
			syslog(LOG_ALERT, "TFTP ERROR %d recvfrom\n",errno);
			return -1;
		}else{
			errx(1,"TFTP ERROR %d recvfrom\n",errno);
		}
	}else{
		return n;
	}
}