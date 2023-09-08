PROJECT = server
CFLAGS = -Wall -g

server: server.o
		$(CC) $(CFLAGS) server.o -o $@

server.o: server.c
		$(CC) $(CFLAGS) -c server.c -o $@

clean:
		-rm -f server *.o *.core