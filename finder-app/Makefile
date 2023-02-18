SHELL=/bin/sh
OBJ=writer
SRC=writer.c
CFLAGS=-Wall -g
CC=aarch64-none-linux-gnu-gcc

all: $(OBJ)

writer: $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(OBJ)
clean: 
	rm -f *.o $(OBJ)

