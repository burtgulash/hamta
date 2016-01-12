CC=gcc
CFLAGS=-std=c99
SRC=$(wildcard *.c)

.PHONY: all test

test: $(SRC)
	$(CC) -o test_runner $^ $(CFLAGS) -g -O0
	./test_runner
