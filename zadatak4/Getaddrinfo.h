#include <syslog.h>
#include <errno.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>


int Getaddrinfo(const char *hostname, const char *service, const struct addrinfo *hints, struct addrinfo **result, int dflag) {
	int erro;
	erro = getaddrinfo(hostname, service, hints, result);
	if (erro<0){
		if (dflag){
			syslog(LOG_ALERT, "TFTP ERROR %d getaddrinfo\n", errno);
			errx(1,"TFTP ERROR %d getaddrinfo\n",errno);
		}else{
			errx(1,"TFTP ERROR %d getaddrinfo\n",errno);
		}
	} else {
		return 0;
	}
}