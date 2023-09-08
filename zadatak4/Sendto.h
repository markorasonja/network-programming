#include <syslog.h>
#include <errno.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>


ssize_t Sendto(int sockfd, void *buff, size_t nbytes, int flags, const struct sockaddr* to, socklen_t addrlen, int deamon_flag){
	int erro;
	erro= sendto(sockfd, buff, nbytes, flags, to, addrlen);
	if (erro<0){
		if (deamon_flag){
			syslog(LOG_ALERT, "TFTP ERROR %d send\n", errno);
			return -1;
		}else{
			errx(1,"TFTP ERROR %d send\n",errno);
		}
	}else{
		return 0;
	} 
}