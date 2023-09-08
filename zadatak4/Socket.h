#include <syslog.h>
#include <errno.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>


int Socket(int family, int type, int protocol, int deamon_flag){
	int n;
	if ((n=socket(family,type,protocol))==-1){
		if (deamon_flag){
			syslog(LOG_ALERT, "TFTP ERROR %d socket\n", errno);
			return -1;
		}else{
			errx(1,"TFTP ERROR %d socket\n",errno);
		}
		
		
	} else {
		return n;
	}
}