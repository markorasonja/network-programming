#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <err.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>


int returnWinner(int tcp_connections[], struct timeval diff[], int delay) {
    struct timeval first, second;
    first.tv_sec = 0;
    first.tv_usec = 0;
    int first_index = -1;
    second.tv_sec = 0;
    second.tv_usec = 0;
    int second_index = -1;
    int i;
    for(i = 0; i < 5; i++){
        if (diff[i].tv_sec == 0 && diff[i].tv_usec == 0)
        {
            continue;
        }
        if (first.tv_sec == 0 && first.tv_usec == 0)
        {
            first = diff[i];
            first_index = i;
            continue;
        }
        if (second.tv_sec == 0 && second.tv_usec == 0)
        {
            second = diff[i];
            second_index = i;
            continue;
        }
        if (first.tv_sec > second.tv_sec || (first.tv_sec == second.tv_sec && first.tv_usec > second.tv_usec))
        {
            struct timeval temp = first;
            first = second;
            second = temp;
            int temp_index = first_index;
            first_index = second_index;
            second_index = temp_index;
        }
    }
    
    return first_index;
}



int main(int argc, char *argv[])
{
    // struct timeval stop, start;
    // gettimeofday(&start, NULL);
    // gettimeofday(&stop, NULL);
    // int diff = (stop.tv_sec - start.tv_sec) * 1000000 + stop.tv_usec - start.tv_usec;


    int udp_port = 5555;
    int tcp_port = 1234;
    int delay = 1000;
    int wait = 10;


    int opt_num = 0;
    int ch;
    while((ch = getopt(argc, argv, "w:d:u:t:")) != -1)
    {
        switch(ch)
        {
            case 'w':
                wait = atoi(optarg);
                opt_num++;
                break;
            case 'd':
                delay = atoi(optarg);
                opt_num++;
                break;
            case 'u':
                udp_port = atoi(optarg);
                opt_num++;
                break;
            case 't':
                tcp_port = atoi(optarg);
                opt_num++;
                break;
            default:
                errx(1, "usage: server [-w wait] [-d delay] [-u udp_port] [-t tcp_port]");
        }
    }


    if(argc != 9)
    {
        errx(1, "usage: server [-w wait] [-d delay] [-u udp_port] [-t tcp_port]");
    }

    int udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if(udp_socket == -1)
    {
        errx(1, "socket error");
    }

    int tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(tcp_socket == -1)
    {
        errx(1, "socket error");
    }

    struct sockaddr_in udp_address;
    udp_address.sin_family = AF_INET;
    udp_address.sin_port = htons(udp_port);
    udp_address.sin_addr.s_addr = htonl(INADDR_ANY);
    if(bind(udp_socket, (struct sockaddr *)&udp_address, sizeof(udp_address)) == -1)
    {
        errx(1, "bind error");
    }

    struct sockaddr_in tcp_address;
    tcp_address.sin_family = AF_INET;
    tcp_address.sin_port = htons(tcp_port);
    tcp_address.sin_addr.s_addr = htonl(INADDR_ANY);
    if(bind(tcp_socket, (struct sockaddr *)&tcp_address, sizeof(tcp_address)) == -1)
    {
        errx(1, "bind error");
    }

    if(listen(tcp_socket, 5) == -1)
    {
        errx(1, "listen error");
    }

    fd_set read_fds, master;
    FD_ZERO(&read_fds);
    FD_ZERO(&master);
    FD_SET(udp_socket, &read_fds);
    FD_SET(tcp_socket, &read_fds);
    FD_SET(udp_socket, &master);

    int max_fd = udp_socket > tcp_socket ? udp_socket : tcp_socket;

    struct timeval timeout;
    timeout.tv_sec = wait;
    timeout.tv_usec = 0;

    int max_tcp_connection = 5;
    int tcp_connections[max_tcp_connection];
    int tcp_connection_count = 0;


    struct timeval diff[5];

    // signal(SIGALRM, SIG_IGN);
    // alarm(wait);

    // forever loop
    for(;;){
        int answered = 0;
        read_fds = master;
        // select
        timeout.tv_sec = wait;
        timeout.tv_usec = 0;
        int ready = select(max_fd + 1, &read_fds, NULL, NULL, &timeout);
        if(ready == -1)
        {
            if (errno = EINTR)
            {
                // reset answered and diff
                printf("tu sam\n");
                answered = 0;
                for(int i = 0; i < 5; i++)
                {
                    diff[i].tv_sec = 0;
                    diff[i].tv_usec = 0;
                }
            }
            else{
                errx(1, "select error");
            }
        }

        if(FD_ISSET(udp_socket, &read_fds))
        {
            printf("new udp message\n");
            char buffer[1024];
            struct sockaddr_in client_address;
            socklen_t client_address_len = sizeof(client_address);
            ssize_t bytes = recvfrom(udp_socket, buffer, sizeof(buffer), 0, (struct sockaddr *)&client_address, &client_address_len);
            if(bytes == -1)
            {
                errx(1, "recvfrom error");
            }

            for(int i = 0; i < tcp_connection_count; i++)
            {
                ssize_t sent = send(tcp_connections[i], buffer, bytes, 0);
                if(sent == -1)
                {
                    errx(1, "send error");
                }
                gettimeofday(&diff[i], NULL);
            }
        }


        if(FD_ISSET(tcp_socket, &read_fds))
        {

            printf("new connection\n");
            struct sockaddr_in client_address;
            socklen_t client_address_len = sizeof(client_address);
            int client_socket = accept(tcp_socket, (struct sockaddr *)&client_address, &client_address_len);
            if(client_socket == -1)
            {
                errx(1, "accept error");
            }



            if (max_tcp_connection == tcp_connection_count) {
                close(client_socket);
                printf("ignoring connection - too many\n");
                continue;
            }

            FD_SET(client_socket, &master);
            if(client_socket > max_fd)
            {
                max_fd = client_socket;
            }

            tcp_connections[tcp_connection_count] = client_socket;
            tcp_connection_count++;
        }
        
        int i;
        for(i = 0; i < tcp_connection_count; i++)
        {
            if(FD_ISSET(tcp_connections[i], &read_fds))
            {
                char buffer[1024];
                ssize_t bytes = recv(tcp_connections[i], buffer, sizeof(buffer), 0);
                if(bytes == -1)
                {
                    errx(1, "recv error");
                }

                if(bytes == 1 && buffer[0] == '\n')
                {
                    struct timeval end;
                    gettimeofday(&end, NULL);
                    int d = (end.tv_sec - diff[i].tv_sec) * 1000000 + end.tv_usec - diff[i].tv_usec;
                    diff[i].tv_sec = d / 1000000;
                    diff[i].tv_usec = d % 1000000;
                    answered++;

                    if(answered == tcp_connection_count)
                    {
                        int winner = returnWinner(tcp_connections, diff, delay);
                        if(winner == -1) {
                            errx(1, "returnWinner error");
                        }
                        
                        struct sockaddr_in fastest_address;
                        socklen_t fastest_address_len = sizeof(fastest_address);
                        if(getpeername(tcp_connections[winner], (struct sockaddr *)&fastest_address, &fastest_address_len) == -1)
                        {
                            errx(1, "getpeername error");
                        }
                        char fastest_address_str[INET_ADDRSTRLEN];
                        inet_ntop(AF_INET, &(fastest_address.sin_addr), fastest_address_str, INET_ADDRSTRLEN);
                        printf("Najbrzi odgovor:     %s:%d, %ld\n", fastest_address_str, ntohs(fastest_address.sin_port), diff[winner].tv_sec);

                        // send the message to all tcp connections
                        for(int j = 0; j < tcp_connection_count; j++)
                        {
                            char msg[100];
                            sprintf(msg, "Najbrzi odgovor:     %s:%d, %ld\n", fastest_address_str, ntohs(fastest_address.sin_port), diff[winner].tv_sec);
                            ssize_t sent = send(tcp_connections[j], msg, strlen(msg), 0);
                            if(sent == -1)
                            {
                                errx(1, "send error");
                            }
                        }
                    }
                }
            }
        }

        if(ready == 0)
        {
            printf("Izjednaceno\n");
            for(i = 0; i < tcp_connection_count; i++)
            {
                printf("tu sam\n");
                if(send(tcp_connections[i], "Izjednaceno", 11, 0) == -1)
                {
                    errx(1, "send error");
                }
            }
            continue;
        }

    }
    return 0;
}