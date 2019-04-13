CC = gcc
CFLAGS = -pipe
PRNAME = simulator

all: simulator.o
	$(CC) $(CFLAGS) $(PRNAME).o -o $(PRNAME)

simulator.o: simulator.c
	$(CC) $(CFLAGS) -c $(PRNAME).c

debug: CFLAGS += -g
debug: all
	@echo DEBUG WITH: gdb $(PRNAME)
	@echo MEMCHECK: valgrind --leak-check=full $(PRNAME) *args*

clean:
	rm $(PRNAME) *.o *.*~ *~
