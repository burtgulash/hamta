CC=gcc
CFLAGS=-std=c99 -O3
SRC=$(wildcard lib/*.c)
INCLUDE=-I./include

.PHONY: clean test

bin/libhamta.so: $(SRC)
	mkdir -p bin
	$(CC) $(INCLUDE) -shared -fpic -o $@ $^ $(CFLAGS) -Wall -Wextra -Wshadow

bin/test_runner: test/test.c $(SRC)
	mkdir -p bin
	$(CC) $(INCLUDE) -o $@ $^ $(CFLAGS) -DDEBUG -ggdb

test: bin/test_runner
	./bin/test_runner

clean:
	rm -r bin
