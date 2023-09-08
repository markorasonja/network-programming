PROJECT = bot UDP_server
CFLAGS = -Wall -g

PRVI_C = bot.c
PRVI_O = ${PRVI_C:.c=.o}

DRUGI_C = UDP_server.c
DRUGI_O = ${DRUGI_C:.c=.o}

all: bot UDP_server

bot: ${PRVI_O}
		$(CC) $(PRVI_O) -o $@

UDP_server: ${DRUGI_O}
		$(CC) $(DRUGI_O) -o $@


clean:
		-rm -f bot UDP_server *.o *.core