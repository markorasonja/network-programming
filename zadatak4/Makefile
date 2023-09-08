PROJECT = tftpserver
SOURCE = tftpserver.c


CC = gcc
CFLAGS = -Wall -g
LDFLAGS =
OBJECTS = ${SOURCE:.c=.o}
HEADERS = Socket.h Error_handle.h Getaddrinfo.h Bind.h Recvfrom.h Sendto.h Deamon_init.h Read_netascii.h

$(PROJECT): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(PROJECT)

$(OBJECTS): $(HEADERS)

clean:
	-rm -f $(PROJECT) $(OBJECTS) *.core
