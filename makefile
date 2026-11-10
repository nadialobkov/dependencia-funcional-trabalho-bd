CC = gcc
CFLAGS = -Wall -Wextra -std=c99

.PHONY: all clean

all: main

main: main.c dpf.c fdtool.c
	$(CC) $(CFLAGS) -o fdtool main.c dpf.c fdtool.c

clean:
	rm -f fdtool