CC = gcc
CFLAGS = -pipe

all: simulator.o
	$(CC) $(CFLAGS) simulator.o -o simulator

simulator.o: simulator.c
	$(CC) $(CFLAGS) -c simulator.c

clean:
	rm simulator *.o *.*~ *~
