CC = gcc
CFLAGS = -Wall -Wextra -std=c99

.PHONY: all clean

all: main

main: main.c dpf.c
	$(CC) $(CFLAGS) -o fdtool main.c dpf.c

clean:
	rm -f fdtool