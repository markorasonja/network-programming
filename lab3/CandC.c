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
#include "PrintHelp.h"
#define MAXLEN 1024


void sig_quit(){
    printf("Server is shutting down...\n");
    exit(0);
}


int main(int argc, char *argv[])
{
    //definiranje portova
    char *udp_port = "5555";
    char *tcp_port = "80";




    //provjera argumenata
    if (argc > 2) errx(1, "Usage: %s [tcp_port]\n", argv[0]);

    if (argc == 2) {
        tcp_port = argv[1];
    }



    //definiranje ruta za http
    struct {
        char *query_command;
    } query_path[] = {
	{"prog_tcp"},  {"prog_tcp_localhost"}, {"prog_udp"}, {"prog_udp_localhost"}, {"run"},  {"run2" },  {"stop"}, {"list"},  {"quit"}};
    int num_query_path = 9;




    //definiranje strukture bot-a
    typedef struct {
        char IP[INET_ADDRSTRLEN];
        char PORT[22];
    } bot;

    typedef struct {
        bot bots[20];
        int num_bots;
    } bot_list;
    


    //definiranje strukture MSG
    typedef struct {
        char IP[INET_ADDRSTRLEN];
        char PORT[22];
    } ip_port;

    typedef struct {
        char command;
        ip_port pairs[20];
        int num_pairs;
    } MSG;




    //definiranje hardkodiranih struktura MSG
    MSG msg_pt;
    msg_pt.command = '1';
    strcpy(msg_pt.pairs[0].IP, "10.0.0.20");
    strcpy(msg_pt.pairs[0].PORT, "1234");
    msg_pt.num_pairs = 1;

    MSG msg_ptl;
    msg_ptl.command = '1';
    strcpy(msg_ptl.pairs[0].IP, "127.0.0.1");
    strcpy(msg_ptl.pairs[0].PORT, "1234");
    msg_ptl.num_pairs = 1;

    MSG msg_pu;
    msg_pu.command = '2';
    strcpy(msg_pu.pairs[0].IP, "10.0.0.20");
    strcpy(msg_pu.pairs[0].PORT, "1234");
    msg_pu.num_pairs = 1;

    MSG msg_pul;
    msg_pul.command = '2';
    strcpy(msg_pul.pairs[0].IP, "127.0.0.1");
    strcpy(msg_pul.pairs[0].PORT, "1234");
    msg_pul.num_pairs = 1;

    MSG msg_r;
    msg_r.command = '3';
    strcpy(msg_r.pairs[0].IP, "127.0.0.1");
    strcpy(msg_r.pairs[0].PORT, "vat");
    strcpy(msg_r.pairs[1].IP, "localhost");
    strcpy(msg_r.pairs[1].PORT, "6789");
    msg_r.num_pairs = 2;

    MSG msg_r2;
    msg_r2.command = '3';
    strcpy(msg_r2.pairs[0].IP, "20.0.0.11");
    strcpy(msg_r2.pairs[0].PORT, "1111");
    strcpy(msg_r2.pairs[1].IP, "20.0.0.12");
    strcpy(msg_r2.pairs[1].PORT, "2222");
    strcpy(msg_r2.pairs[2].IP, "20.0.0.13");
    strcpy(msg_r2.pairs[2].PORT, "hostmon");
    msg_r2.num_pairs = 3;

    MSG msg_stop;
    msg_stop.command = '4';

    MSG msg_quit;
    msg_quit.command = '0';

    char *NEPOZNATA = "NEPOZNATA";




    //otvaranje socketa na udp portu 5555
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; //IPv4
    hints.ai_socktype = SOCK_DGRAM; //UDP socket
    hints.ai_flags = AI_PASSIVE; //localhost
    
    if( getaddrinfo(NULL, udp_port, &hints, &res) != 0)
    {
        errx(1, "getaddrinfo error");
    }

    int udp_sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if(udp_sock == -1) errx(1, "socket error");
    if(setsockopt(udp_sock, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int)) == -1) errx(1, "setsockopt error");
    if(bind(udp_sock, res->ai_addr, res->ai_addrlen) == -1) errx(1, "bind error 5555: %s", strerror(errno));
    freeaddrinfo(res);




    //otvaranje tcp socketa na zadanom portu
    hints.ai_family = AF_INET; //IPv4
    hints.ai_socktype = SOCK_STREAM; //TCP socket
    hints.ai_flags = AI_PASSIVE; //localhost

    if( getaddrinfo(NULL, tcp_port, &hints, &res) != 0) errx(1, "getaddrinfo error");
    int tcp_sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if(tcp_sock == -1) errx(1, "socket error");
    if(setsockopt(tcp_sock, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int)) == -1) errx(1, "setsockopt error");
    if(bind(tcp_sock, res->ai_addr, res->ai_addrlen) == -1) errx(1, "bind error tcp");
    if(listen(tcp_sock, 10) == -1) errx(1, "listen error");
    freeaddrinfo(res);




    //postavljanje SELECT-a
    fd_set master;
    fd_set read_set;
    int max_fd = 0;
    FD_ZERO(&master); //praznjenje seta
    FD_ZERO(&read_set);
    FD_SET(udp_sock, &master); //dodavanje socketa u set
    FD_SET(tcp_sock, &master);
    FD_SET(STDIN_FILENO, &master);
    max_fd = tcp_sock > udp_sock ? tcp_sock : udp_sock; //odredivanje max socketa




    //definiranje potrebnih struktura prije glavne petlje
    char buffer[MAXLEN];
    memset(buffer, 0, MAXLEN);
    struct sockaddr_in client;
    socklen_t client_len;
    bot_list bots;
    bots.num_bots = 0;
    int http_sock;
    pid_t parrent_pid = getpid();
    signal(SIGQUIT, sig_quit);


    //glavna petlja
    for(;;)
    {   
        read_set = master; //postavljanje read_seta na master set
        if(select(max_fd + 1, &read_set, NULL, NULL, NULL) == -1) errx(1, "select error"); //select

        //provjera da li je dosao zahtjev na stdin
        if(FD_ISSET(STDIN_FILENO, &read_set))
        {
            fgets(buffer, 10, stdin);
            char command[3];
            strncpy(command, buffer, strlen(buffer));
            command[strlen(buffer)] = '\0'; //maknuti \n sa kraja stringa

            printf("Unesena je naredba: %s\n", command);

            //loop za provjeru naredbi
            if (strncmp(command, "pt\n", strlen(command)) == 0){
                for (int i = 0; i < bots.num_bots; i++)
                {
                    struct sockaddr_in bot_addr;
                    bot_addr.sin_family = AF_INET;
                    bot_addr.sin_port = htons(atoi(bots.bots[i].PORT));
                    if(inet_aton(bots.bots[i].IP, &bot_addr.sin_addr) == 0) errx(1, "inet_aton error");
                    //send msg_pt
                    int size = 1 + msg_pt.num_pairs * sizeof(ip_port);
                    if(sendto(udp_sock, &msg_pt, size, 0, (struct sockaddr *)&bot_addr, sizeof(bot_addr)) == -1) errx(1, "sendto error");
                }
            }
            else if (strncmp(command, "ptl\n", strlen(command)) == 0){
                for (int i = 0; i < bots.num_bots; i++)
                {
                    struct sockaddr_in bot_addr;
                    bot_addr.sin_family = AF_INET;
                    bot_addr.sin_port = htons(atoi(bots.bots[i].PORT));
                    if(inet_aton(bots.bots[i].IP, &bot_addr.sin_addr) == 0) errx(1, "inet_aton error");
                    //send msg_plt
                    int size = 1 + msg_ptl.num_pairs * sizeof(ip_port);
                    if(sendto(udp_sock, &msg_ptl, size, 0, (struct sockaddr *)&bot_addr, sizeof(bot_addr)) == -1) errx(1, "sendto error");
                }
            }
            else if (strncmp(command, "pu\n", strlen(command)) == 0){
                for (int i = 0; i < bots.num_bots; i++)
                {
                    struct sockaddr_in bot_addr;
                    bot_addr.sin_family = AF_INET;
                    bot_addr.sin_port = htons(atoi(bots.bots[i].PORT));
                    if(inet_aton(bots.bots[i].IP, &bot_addr.sin_addr) == 0) errx(1, "inet_aton error");
                    //send msg_pu
                    int size = 1 + msg_pu.num_pairs * sizeof(ip_port);
                    if(sendto(udp_sock, &msg_pu, size, 0, (struct sockaddr *)&bot_addr, sizeof(bot_addr)) == -1) errx(1, "sendto error");
                }
            }
            else if (strncmp(command, "pul\n", strlen(command)) == 0){
                for (int i = 0; i < bots.num_bots; i++)
                {
                    struct sockaddr_in bot_addr;
                    bot_addr.sin_family = AF_INET;
                    bot_addr.sin_port = htons(atoi(bots.bots[i].PORT));
                    if(inet_aton(bots.bots[i].IP, &bot_addr.sin_addr) == 0) errx(1, "inet_aton error");
                    //send msg_pul
                    int size = 1 + msg_pul.num_pairs * sizeof(ip_port);
                    if(sendto(udp_sock, &msg_pul, size, 0, (struct sockaddr *)&bot_addr, sizeof(bot_addr)) == -1) errx(1, "sendto error");
                }
            }
            else if (strncmp(command, "r\n", strlen(command)) == 0){
                for (int i = 0; i < bots.num_bots; i++)
                {
                    struct sockaddr_in bot_addr;
                    bot_addr.sin_family = AF_INET;
                    bot_addr.sin_port = htons(atoi(bots.bots[i].PORT));
                    if(inet_aton(bots.bots[i].IP, &bot_addr.sin_addr) == 0) errx(1, "inet_aton error");
                    //send msg_r
                    int size = 1 + msg_r.num_pairs * sizeof(ip_port);
                    if(sendto(udp_sock, (const void*)&msg_r, size, 0, (struct sockaddr *)&bot_addr, sizeof(bot_addr)) == -1) errx(1, "sendto error");
                }
            }
            else if (strncmp(command, "r2\n", strlen(command)) == 0){
                for (int i = 0; i < bots.num_bots; i++)
                {
                    struct sockaddr_in bot_addr;
                    bot_addr.sin_family = AF_INET;
                    bot_addr.sin_port = htons(atoi(bots.bots[i].PORT));
                    if(inet_aton(bots.bots[i].IP, &bot_addr.sin_addr) == 0) errx(1, "inet_aton error");
                    //send msg_r2
                    int size = 1 + msg_r2.num_pairs * sizeof(ip_port);
                    if(sendto(udp_sock, (const void*)&msg_r2, size, 0, (struct sockaddr *)&bot_addr, sizeof(bot_addr)) == -1) errx(1, "sendto error");
                }
            }
            else if (strncmp(command, "s\n", strlen(command)) == 0){
                for (int i = 0; i < bots.num_bots; i++)
                {
                    struct sockaddr_in bot_addr;
                    bot_addr.sin_family = AF_INET;
                    bot_addr.sin_port = htons(atoi(bots.bots[i].PORT));
                    if(inet_aton(bots.bots[i].IP, &bot_addr.sin_addr) == 0) errx(1, "inet_aton error");
                    //send msg_stop
                    if(sendto(udp_sock, (const void*)&msg_stop, 1, 0, (struct sockaddr *)&bot_addr, sizeof(bot_addr)) == -1) errx(1, "sendto error");
                }
            }
            else if (strncmp(command, "q\n", strlen(command)) == 0){
                //sending quit message to all bots
                for (int i = 0; i < bots.num_bots; i++)
                {
                    struct sockaddr_in bot_addr;
                    bot_addr.sin_family = AF_INET;
                    bot_addr.sin_port = htons(atoi(bots.bots[i].PORT));
                    if(inet_aton(bots.bots[i].IP, &bot_addr.sin_addr) == 0) errx(1, "inet_aton error");
                    //send msg_quit
                    if(sendto(udp_sock, (const void*)&msg_quit, 1, 0, (struct sockaddr *)&bot_addr, sizeof(bot_addr)) == -1) errx(1, "sendto error");
                }
                printf("Server is shutting down...\n");
                close(udp_sock);
                exit(0);
            }
            else if (strncmp(command, "h\n", strlen(command)) == 0){
                PrintHelp();
            }
            else if (strncmp(command, "l\n", strlen(command)) == 0){
                for (int i = 0; i < bots.num_bots; i++)
                {
                    printf("Bot[%d]-> IP: %s, port: %s\n", i, bots.bots[i].IP, bots.bots[i].PORT);
                }
                printf("\n");
            }
            else if (strncmp(command, "n\n", strlen(command)) == 0){
                for (int i = 0; i < bots.num_bots; i++)
                {
                    struct sockaddr_in bot_addr;
                    bot_addr.sin_family = AF_INET;
                    bot_addr.sin_port = htons(atoi(bots.bots[i].PORT));
                    if(inet_aton(bots.bots[i].IP, &bot_addr.sin_addr) == 0) errx(1, "inet_aton error");
                    //send NEPOZNATA
                    if(sendto(udp_sock, (const void*)&NEPOZNATA, sizeof(NEPOZNATA), 0, (struct sockaddr *)&bot_addr, sizeof(bot_addr)) == -1) errx(1, "sendto error");
                }
            }
            else
            {
                printf("Nepoznata naredba\n");
            }


            //pogledati koja naredba je unesena
            //izvrsiti to
            //memset(buffer, 0, MAXLEN); //praznjenje buffera
        }

        //provjera da li je dosao zahtjev na udp socket
        if(FD_ISSET(udp_sock, &read_set))
        {
            client_len = sizeof(client);
            int len = recvfrom(udp_sock, buffer, MAXLEN, 0, (struct sockaddr*)&client, &client_len);
            if(len == -1) errx(1, "recvfrom error");

            //pogledati koja naredba je unesena
            buffer[len] = '\0';

            //prihvaca se jedino REG\n
            if (strcmp(buffer, "REG\n") == 0)
            {
                bot new_bot;
                strcpy(new_bot.IP, inet_ntoa(client.sin_addr));
                char port[6];
                sprintf(port, "%d", ntohs(client.sin_port));
                strcpy(new_bot.PORT, port);

                if(bots.num_bots == 20)
                {
                    printf("Dostignut maksimalan broj botova\n");
                    continue;
                }

                bots.bots[bots.num_bots] = new_bot;
                bots.num_bots++;
                printf("Bot %s:%s registriran\n", new_bot.IP, new_bot.PORT);
                //provjeriti da li je vec registriran
                //ako nije, registrirati ga
                //ako je, poslati mu poruku da je vec registriran
            }
            else
            {
                printf("Bot mora poslati REG\n");
            }
            memset(buffer, 0, MAXLEN); //praznjenje buffera
        }

        //provjera da li je dosao zahtjev na tcp socket
        if(FD_ISSET(tcp_sock, &read_set))
        {
            client_len = sizeof(client);
            http_sock = accept(tcp_sock, (struct sockaddr*)&client, &client_len);
            if(http_sock == -1) errx(1, "accept error");

            pid_t pid = fork();
            if(pid == -1) errx(1, "fork error");
            if(pid != 0)
            {
                close(http_sock);
            }
            else
            {
                close(tcp_sock);
                //close(udp_sock);

                //handle HTTP request (only GET)
                char http_buffer[MAXLEN * 4];
                char *method;
                char *path;
                memset(http_buffer, 0, MAXLEN * 4);
                int len = read(http_sock, http_buffer, MAXLEN * 4);
                if(len == -1) errx(1, "read error");

                method = strtok(http_buffer, " ");
                printf("Method: %s\n", method);
                path = strtok(NULL, " ");
                printf("Path: %s\n", path);
                if(strcmp(method, "GET") != 0)
                {
                    //send method not allowed
                    char *msg = "HTTP/1.1 405 Method Not Allowed\r\n\r\n";
                    if(write(http_sock, msg, strlen(msg)) == -1) errx(1, "write error");
                    close(http_sock);
                    printf("Method not allowed\n");
                    exit(0);
                }

                //chech if path is some file in current directory
                char *file_path = malloc(strlen(path) + 2);
                strcpy(file_path, ".");
                strcat(file_path, path);
                if(access(file_path, F_OK) == -1)
                {
                    char *found_path;
                    //chech if path starts with /bot and have path from struct query_path
                    if(strncmp(path, "/bot", 4) == 0)
                    {   
                        printf("Path starts with /bot\n");
                        //remove /bot
                        char *bot_path = malloc(strlen(path) + 2);
                        strcat(bot_path, path + 5);

                        printf("Bot path: %s\n", bot_path);

                        //check if bot_path is in query_path
                        int found = 0;
                        for (int i = 0; i < num_query_path; i++)
                        {
                            if(strcmp(bot_path, query_path[i].query_command) == 0)
                            {
                                found = 1;
                                found_path = query_path[i].query_command;
                                break;
                            }
                        }

                        if(found)
                        {
                            
                            if(strncmp(found_path, "prog_tcp", strlen(found_path)) == 0)
                            {
                                char *msg = "HTTP/1.1 200 OK\r\n\r\n";
                                if(write(http_sock, msg, strlen(msg)) == -1) errx(1, "write error");
                                for (int i = 0; i < bots.num_bots; i++)
                                {
                                    struct sockaddr_in bot_addr;
                                    bot_addr.sin_family = AF_INET;
                                    bot_addr.sin_port = htons(atoi(bots.bots[i].PORT));
                                    if(inet_aton(bots.bots[i].IP, &bot_addr.sin_addr) == 0) errx(1, "inet_aton error");
                                    //send msg_pt
                                    int size = 1 + msg_pt.num_pairs * sizeof(ip_port);
                                    if(sendto(udp_sock, &msg_pt, size, 0, (struct sockaddr *)&bot_addr, sizeof(bot_addr)) == -1) errx(1, "sendto error");
                                }
                            }
                            else if(strncmp(found_path, "prog_tcp_localhost", strlen(found_path)) == 0)
                            {
                                char *msg = "HTTP/1.1 200 OK\r\n\r\n";
                                if(write(http_sock, msg, strlen(msg)) == -1) errx(1, "write error");
                                for (int i = 0; i < bots.num_bots; i++)
                                {
                                    struct sockaddr_in bot_addr;
                                    bot_addr.sin_family = AF_INET;
                                    bot_addr.sin_port = htons(atoi(bots.bots[i].PORT));
                                    if(inet_aton(bots.bots[i].IP, &bot_addr.sin_addr) == 0) errx(1, "inet_aton error");
                                    //send msg_plt
                                    int size = 1 + msg_ptl.num_pairs * sizeof(ip_port);
                                    if(sendto(udp_sock, &msg_ptl, size, 0, (struct sockaddr *)&bot_addr, sizeof(bot_addr)) == -1) errx(1, "sendto error");
                                }
                            }
                            else if(strncmp(found_path, "prog_udp", strlen(found_path)) == 0)
                            {
                                char *msg = "HTTP/1.1 200 OK\r\n\r\n";
                                if(write(http_sock, msg, strlen(msg)) == -1) errx(1, "write error");
                                for (int i = 0; i < bots.num_bots; i++)
                                {
                                    struct sockaddr_in bot_addr;
                                    bot_addr.sin_family = AF_INET;
                                    bot_addr.sin_port = htons(atoi(bots.bots[i].PORT));
                                    if(inet_aton(bots.bots[i].IP, &bot_addr.sin_addr) == 0) errx(1, "inet_aton error");
                                    //send msg_pu
                                    int size = 1 + msg_pu.num_pairs * sizeof(ip_port);
                                    if(sendto(udp_sock, &msg_pu, size, 0, (struct sockaddr *)&bot_addr, sizeof(bot_addr)) == -1) errx(1, "sendto error");
                                }
                            }
                            else if(strncmp(found_path, "prog_udp_localhost", strlen(found_path)) == 0)
                            {
                                char *msg = "HTTP/1.1 200 OK\r\n\r\n";
                                if(write(http_sock, msg, strlen(msg)) == -1) errx(1, "write error");
                                for (int i = 0; i < bots.num_bots; i++)
                                {
                                    struct sockaddr_in bot_addr;
                                    bot_addr.sin_family = AF_INET;
                                    bot_addr.sin_port = htons(atoi(bots.bots[i].PORT));
                                    if(inet_aton(bots.bots[i].IP, &bot_addr.sin_addr) == 0) errx(1, "inet_aton error");
                                    //send msg_pul
                                    int size = 1 + msg_pul.num_pairs * sizeof(ip_port);
                                    if(sendto(udp_sock, &msg_pul, size, 0, (struct sockaddr *)&bot_addr, sizeof(bot_addr)) == -1) errx(1, "sendto error");
                                }
                            }
                            else if(strncmp(found_path, "run", strlen(found_path)) == 0)
                            {
                                char *msg = "HTTP/1.1 200 OK\r\n\r\n";
                                if(write(http_sock, msg, strlen(msg)) == -1) errx(1, "write error");
                                for (int i = 0; i < bots.num_bots; i++)
                                {
                                    struct sockaddr_in bot_addr;
                                    bot_addr.sin_family = AF_INET;
                                    bot_addr.sin_port = htons(atoi(bots.bots[i].PORT));
                                    if(inet_aton(bots.bots[i].IP, &bot_addr.sin_addr) == 0) errx(1, "inet_aton error");
                                    //send msg_r
                                    int size = 1 + msg_r.num_pairs * sizeof(ip_port);
                                    if(sendto(udp_sock, (const void*)&msg_r, size, 0, (struct sockaddr *)&bot_addr, sizeof(bot_addr)) == -1) errx(1, "sendto error");
                                }
                            }
                            else if(strncmp(found_path, "run2", strlen(found_path)) == 0)
                            {
                                char *msg = "HTTP/1.1 200 OK\r\n\r\n";
                                if(write(http_sock, msg, strlen(msg)) == -1) errx(1, "write error");
                                for (int i = 0; i < bots.num_bots; i++)
                                {
                                    struct sockaddr_in bot_addr;
                                    bot_addr.sin_family = AF_INET;
                                    bot_addr.sin_port = htons(atoi(bots.bots[i].PORT));
                                    if(inet_aton(bots.bots[i].IP, &bot_addr.sin_addr) == 0) errx(1, "inet_aton error");
                                    //send msg_r2
                                    int size = 1 + msg_r2.num_pairs * sizeof(ip_port);
                                    if(sendto(udp_sock, (const void*)&msg_r2, size, 0, (struct sockaddr *)&bot_addr, sizeof(bot_addr)) == -1) errx(1, "sendto error");
                                }
                            }
                            else if(strncmp(found_path, "stop", strlen(found_path)) == 0)
                            {
                                char *msg = "HTTP/1.1 200 OK\r\n\r\n";
                                if(write(http_sock, msg, strlen(msg)) == -1) errx(1, "write error");
                                for (int i = 0; i < bots.num_bots; i++)
                                {
                                    struct sockaddr_in bot_addr;
                                    bot_addr.sin_family = AF_INET;
                                    bot_addr.sin_port = htons(atoi(bots.bots[i].PORT));
                                    if(inet_aton(bots.bots[i].IP, &bot_addr.sin_addr) == 0) errx(1, "inet_aton error");
                                    //send msg_stop
                                    if(sendto(udp_sock, (const void*)&msg_stop, 1, 0, (struct sockaddr *)&bot_addr, sizeof(bot_addr)) == -1) errx(1, "sendto error");
                                }
                            }
                            else if(strncmp(found_path, "list", strlen(found_path)) == 0)
                            {
                                //send list of bots to as http response
                                char *msg = "HTTP/1.1 200 OK\r\n\r\n";
                                if(write(http_sock, msg, strlen(msg)) == -1) errx(1, "write error");
                                char *list = malloc(1000);
                                strcpy(list, "List of bots:\n");
                                for (int i = 0; i < bots.num_bots; i++)
                                {
                                    strcat(list, bots.bots[i].IP);
                                    strcat(list, ":");
                                    strcat(list, bots.bots[i].PORT);
                                    strcat(list, "\n");
                                }
                                if(write(http_sock, list, strlen(list)) == -1) errx(1, "write error");
                                free(list);
                            }
                            else if(strncmp(found_path, "quit", strlen(found_path)) == 0)
                            {
                                char *msg = "HTTP/1.1 200 OK\r\n\r\n";
                                if(write(http_sock, msg, strlen(msg)) == -1) errx(1, "write error");
                                for (int i = 0; i < bots.num_bots; i++)
                                {
                                    struct sockaddr_in bot_addr;
                                    bot_addr.sin_family = AF_INET;
                                    bot_addr.sin_port = htons(atoi(bots.bots[i].PORT));
                                    if(inet_aton(bots.bots[i].IP, &bot_addr.sin_addr) == 0) errx(1, "inet_aton error");
                                    //send msg_quit
                                    if(sendto(udp_sock, (const void*)&msg_quit, 1, 0, (struct sockaddr *)&bot_addr, sizeof(bot_addr)) == -1) errx(1, "sendto error");
                                }
                                kill(parrent_pid, SIGQUIT);
                                exit(0);
                            }
                            close(http_sock);
                            exit(0);
                        }
                    }
                    //send not found
                    char *msg = "HTTP/1.1 404 Not Found\r\n\r\n";
                    if(write(http_sock, msg, strlen(msg)) == -1) errx(1, "write error");
                    close(http_sock);
                    printf("File not found\n");
                    exit(0);
                }

                //send file
                FILE *file = fopen(file_path, "r");
                if(file == NULL) errx(1, "fopen error");
                char *msg = "HTTP/1.1 200 OK\r\n\r\n";
                if(write(http_sock, msg, strlen(msg)) == -1) errx(1, "write error");
                char file_buffer[MAXLEN];
                memset(file_buffer, 0, MAXLEN);
                while(fgets(file_buffer, MAXLEN, file) != NULL)
                {
                    if(write(http_sock, file_buffer, strlen(file_buffer)) == -1) errx(1, "write error");
                    memset(file_buffer, 0, MAXLEN);
                }
                fclose(file);
                close(http_sock);
                exit(0);
            }
            memset(buffer, 0, MAXLEN); //praznjenje buffera
        }
    }
    return 0;
}