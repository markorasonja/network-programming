PROJEKT = tcpserver tcpklijent
CFLAGS = -Wall -g
LDFLAGS =
POMOCNI_C = pomocni1.c pomocni2.c
POMOCNI_O = ${POMOCNI_C:.c=.o}
HEADERS = Socket.h

PRVI_C = tcpserver.c
PRVI_O = ${PRVI_C:.c=.o}

DRUGI_C = tcpklijent.c
DRUGI_O = ${DRUGI_C:.c=.o}

all: tcpserver tcpklijent

tcpserver: ${PRVI_O}
	$(CC) $(PRVI_O) -o $@

tcpklijent: ${DRUGI_O}
	$(CC) $(DRUGI_O) -o $@

${PRVI_O}: ${HEADERS} 

clean:
	-rm -f tcpserver tcpklijent *.o *.core