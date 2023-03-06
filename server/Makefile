SHELL=/bin/sh
OBJ=aesdsocket
SRC=aesdsocket.c
CFLAGS=-Wall -Werror -g
CC=aarch64-none-linux-gnu-gcc
COMPILE=gcc

all: $(OBJ)

writer: $(SRC)
	$(COMPILE) $(CFLAGS) $(SRC) -o $(OBJ)
clean: 
	rm -f *.o $(OBJ)

