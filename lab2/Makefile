PROJECT = bot server
CFLAGS = -Wall -g

PRVI_C = bot.c
PRVI_O = ${PRVI_C:.c=.o}

DRUGI_C = server.c
DRUGI_O = ${DRUGI_C:.c=.o}

all: bot server

bot: ${PRVI_O}
		$(CC) $(PRVI_O) -o $@

server: ${DRUGI_O}
		$(CC) $(DRUGI_O) -o $@


clean:
		-rm -f bot server *.o *.core