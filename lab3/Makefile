PROJECT = CandC bot server
CFLAGS = -Wall -g

HEADERS = PrintHelp.h

NULTI_C = CandC.c
NULTI_O = ${NULTI_C:.c=.o}

PRVI_C = bot.c
PRVI_O = ${PRVI_C:.c=.o}

DRUGI_C = server.c
DRUGI_O = ${DRUGI_C:.c=.o}

all: CandC bot server

CandC: ${NULTI_O}
		$(CC) $(NULTI_O) -o $@

bot: ${PRVI_O}
		$(CC) $(PRVI_O) -o $@

server: ${DRUGI_O}
		$(CC) $(DRUGI_O) -o $@

${NULTI_O}: ${HEADERS}


clean:
		-rm -f CandC bot server *.o *.core