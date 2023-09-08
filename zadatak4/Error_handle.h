#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <unistd.h>


void Error_handle(int e, int deamon_flag)
{
    if( deamon_flag == 1)
    {
        syslog(LOG_WARNING, "TFTP ERROR %d error RRQ", e );
    }
    else
    {
        switch (e){
            case 0: fprintf(stderr, "Nedefinirani error"); break;
            case 1: fprintf(stderr, "File not found"); break;  
            case 2: fprintf(stderr, "Access violation"); break;  
            case 3: fprintf(stderr, "Disk full"); break;  
            case 4: fprintf(stderr, "Illegal TFTP operation"); break;  
            case 5: fprintf(stderr, "Unknown port"); break;  
            case 6: fprintf(stderr, "File already exists"); break;  
            case 7: fprintf(stderr, "No such user"); break;    
        }
    }
}