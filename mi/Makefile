PROJEKT = server klijent
CFLAGS = -Wall -g
LDFLAGS =
POMOCNI_C = pomocni1.c pomocni2.c
POMOCNI_O = ${POMOCNI_C:.c=.o}
HEADERS = Socket.h Getaddrinfo.h

PRVI_C = server.c
PRVI_O = ${PRVI_C:.c=.o}

DRUGI_C = klijent.c
DRUGI_O = ${DRUGI_C:.c=.o}

all: server klijent

server: ${PRVI_O}
	$(CC) $(PRVI_O) -o $@

klijent: ${DRUGI_O}
	$(CC) $(DRUGI_O) -o $@

${PRVI_O}: ${HEADERS} 

clean:
	-rm -f server klijent *.o *.core