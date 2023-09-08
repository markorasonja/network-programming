PROJECT = prog
SOURCE = prog.c

OBJECTS = ${SOURCE:.c=.o}

$(PROJECT): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(PROJECT)

clean:
	-rm -f $(PROJECT) $(OBJECTS) *.core
