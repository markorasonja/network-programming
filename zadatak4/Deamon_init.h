#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <syslog.h>
#include <signal.h>
#include <unistd.h>


int Deamon_init(const char *pname, int facility){
	int i;
	pid_t pid;
	
	if ((pid = fork()) < 0)
		return (-1);
	else if (pid)
		_exit(0);
	
	if (setsid() < 0)
		return (-1);
	
	signal (SIGHUP, SIG_IGN);
	
	if ((pid = fork())< 0 )
		return (-1);	
	else if (pid)
		_exit(0);
	
    chdir ("/tftpboot");
	
	for (i=0; i<64; i++)
		close (i);
	
	open("/dev/null", O_RDONLY);
	open("/dev/null", O_RDWR);
	open("/dev/null", O_RDWR);	
	
	openlog(pname, LOG_PID, LOG_FTP);
	return 0;
}