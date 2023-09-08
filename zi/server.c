#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <string.h>
#include <netdb.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>



int returnWinner(int tcp_connections[], int diff[], int delay, int tcp_connection_count) {
    int first_index = 0;
    int second_index = 1;
    int i;

    if (tcp_connection_count == 1) {
        return 0;
    }

    for(i = 1; i < tcp_connection_count; i++)
    {
        if(diff[i] < diff[first_index])
        {
            second_index = first_index;
            first_index = i;
        }
        else if(diff[i] < diff[second_index])
        {
            second_index = i;
        }
    }

    if (diff[first_index] - diff[second_index] < delay) {
        return -1;
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

    char *udpPort = argv[6];
    char *tcpPort = argv[8];

    //otvaranje socketa na udp portu
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; //IPv4
    hints.ai_socktype = SOCK_DGRAM; //UDP socket
    hints.ai_flags = AI_PASSIVE; //localhost
    
    if( getaddrinfo(NULL, udpPort, &hints, &res) != 0)
    {
        errx(1, "getaddrinfo error");
    }

    int udp_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if(udp_socket == -1) errx(1, "socket error");
    if(setsockopt(udp_socket, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int)) == -1) errx(1, "setsockopt error");
    if(bind(udp_socket, res->ai_addr, res->ai_addrlen) == -1) errx(1, "bind error 5555: %s", strerror(errno));
    freeaddrinfo(res);




    //otvaranje tcp socketa na zadanom portu
    hints.ai_family = AF_INET; //IPv4
    hints.ai_socktype = SOCK_STREAM; //TCP socket
    hints.ai_flags = AI_PASSIVE; //localhost

    if( getaddrinfo(NULL, tcpPort, &hints, &res) != 0) errx(1, "getaddrinfo error");
    int tcp_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if(tcp_socket == -1) errx(1, "socket error");
    if(setsockopt(tcp_socket, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int)) == -1) errx(1, "setsockopt error");
    if(bind(tcp_socket, res->ai_addr, res->ai_addrlen) == -1) errx(1, "bind error tcp");
    if(listen(tcp_socket, 5) == -1) errx(1, "listen error");
    freeaddrinfo(res);

    fd_set read_fds, master;
    FD_ZERO(&read_fds);
    FD_ZERO(&master);
    FD_SET(udp_socket, &read_fds);
    FD_SET(tcp_socket, &read_fds);
    FD_SET(udp_socket, &master);
    FD_SET(tcp_socket, &master);

    int max_fd = udp_socket > tcp_socket ? udp_socket : tcp_socket;

    struct timeval timeout;
    timeout.tv_sec = wait;
    timeout.tv_usec = 0;

    int max_tcp_connection = 5;
    int tcp_connections[max_tcp_connection];
    int tcp_connection_count = 0;


    int diff[5];
    struct timeval start[5];
    struct timeval stop[5];

    // signal(SIGALRM, SIG_IGN);
    // alarm(wait);

    // forever loop
    for(;;){
        int answered = 0;
        int udp_answered = 0;
        int accepting = 0;
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
                    diff[i] = 0;
                }
                answered = 0;
                accepting = 0;
                // alarm(wait);
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
                gettimeofday(&start[i], NULL);
            }
            // postavljanje da se moze primati tcp
            accepting = 1;
            udp_answered = 1;
        }


        if(FD_ISSET(tcp_socket, &read_fds))
        {

            printf("new tcp connection\n");
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

                //if buffer has \n
                char *str = strchr(buffer, '\n');
                if(str != NULL)
                {
                    gettimeofday(&stop[i], NULL);
                    int d = (stop[i].tv_sec - start[i].tv_sec) * 1000000 + stop[i].tv_usec - start[i].tv_usec;
                    diff[i] = d;
                    answered++;

                    if(answered == tcp_connection_count)
                    {
                        char msg[100];
                        int winner = returnWinner(tcp_connections, diff, delay, tcp_connection_count);
                        if(winner == -1) {
                            // izjednaceno
                            printf("Izjednaceno\n");
                            sprintf(msg, "Izjednaceno\n");
                        }

                        else{
                            struct sockaddr_in fastest_address;
                            socklen_t fastest_address_len = sizeof(fastest_address);
                            if(getpeername(tcp_connections[winner], (struct sockaddr *)&fastest_address, &fastest_address_len) == -1)
                            {
                                errx(1, "getpeername error");
                            }
                            char fastest_address_str[INET_ADDRSTRLEN];
                            inet_ntop(AF_INET, &(fastest_address.sin_addr), fastest_address_str, INET_ADDRSTRLEN);
                            printf("Najbrzi odgovor:     %s:%d, %d\n", fastest_address_str, ntohs(fastest_address.sin_port), diff[winner]);
                            sprintf(msg, "Najbrzi odgovor:     %s:%d, %d\n", fastest_address_str, ntohs(fastest_address.sin_port), diff[winner]);
                        }

                        for(int j = 0; j < tcp_connection_count; j++)
                        {
                            if(send(tcp_connections[j], msg, strlen(msg), 0) == -1)
                            {
                                errx(1, "send error");
                            }
                        }
                        accepting = 0;
                        answered = 0;
                    }
                }
            }
        }

        if(ready == 0)
        {
            if (answered == 0) {
                printf("Izjednaceno\n");
                for(i = 0; i < tcp_connection_count; i++)
                {
                    printf("tu sam\n");
                    if(send(tcp_connections[i], "Izjednaceno", 11, 0) == -1)
                    {
                        errx(1, "send error");
                    }
                }
                accepting = 0;
                answered = 0;
            }
            else {
                char msg[100];
                int winner = returnWinner(tcp_connections, diff, delay, tcp_connection_count);
                if(winner == -1) {
                    // izjednaceno
                    printf("Izjednaceno\n");
                    sprintf(msg, "Izjednaceno\n");
                }

                else{
                    struct sockaddr_in fastest_address;
                    socklen_t fastest_address_len = sizeof(fastest_address);
                    if(getpeername(tcp_connections[winner], (struct sockaddr *)&fastest_address, &fastest_address_len) == -1)
                    {
                        errx(1, "getpeername error");
                    }
                    char fastest_address_str[INET_ADDRSTRLEN];
                    inet_ntop(AF_INET, &(fastest_address.sin_addr), fastest_address_str, INET_ADDRSTRLEN);
                    printf("Najbrzi odgovor:     %s:%d, %d\n", fastest_address_str, ntohs(fastest_address.sin_port), diff[winner]);
                    sprintf(msg, "Najbrzi odgovor:     %s:%d, %d\n", fastest_address_str, ntohs(fastest_address.sin_port), diff[winner]);
                }

                for(int j = 0; j < tcp_connection_count; j++)
                {
                    if(send(tcp_connections[j], msg, strlen(msg), 0) == -1)
                    {
                        errx(1, "send error");
                    }
                }
                accepting = 0;
                answered = 0;
            }
        }
    }
    return 0;
}