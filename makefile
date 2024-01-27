CC = gcc
OBJS = mainmat.o mymat.o
EXEC = mainmat
DEBUG_FLAG = -g
COMP_FLAG = -Wall -ansi -pedantic $(DEBUG_FLAG)

$(EXEC): $(OBJS)
	$(CC) $(DEBUG_FLAG) $(OBJS) -o $@

mainmat.o: mainmat.c mymat.h
	$(CC) -c $(COMP_FLAG) $*.c

mymat.o: mymat.c mymat.h
	$(CC) -c $(COMP_FLAG) $*.c

clean:
	rm -f $(OBJS)
