#include <stdio.h>
#include <syslog.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>


int Read_netascii(char *buffer, FILE *f, int *flag, int *novired, int *zastavica_za_novi_red){
	int i=0;
	int broj=0;
	char znak;
	if (*flag == 1 ){
		buffer[i++] = '\n';
		*novired = *novired+1;
		// broj = 2;
		broj = 1;
		// zastava_za_novi_red=1;
	}
	while(broj++ < 512){
		if (feof(f) !=0) {
			return broj;
		}
		znak = fgetc(f);
		if(znak == '\n'){
			*novired=*novired+1;
			buffer[i++] = '\r';
			if(++broj >= 512){
				*flag = 1;
				return 512;
			}
		}
		buffer[i++] = znak;
	}
	*flag=0;
	return 512;
}
