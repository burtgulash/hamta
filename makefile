CC=gcc
CFLAGS=-std=c99 -O3 -Wall -Wextra -Wshadow
SRC=$(wildcard lib/*.c)
INCLUDE=-I./include

.PHONY: clean test

bin/libhamta.so: $(SRC)
	mkdir -p bin
	$(CC) $(INCLUDE) -shared -fpic -o $@ $^ $(CFLAGS)

bin/test_runner: test/test.c bin/libhamta.so
	$(CC) $(INCLUDE) -L./bin -Wl,-rpath=./bin -o $@ $< -lhamta $(CFLAGS) -DDEBUG

test: bin/test_runner
	./bin/test_runner

clean:
	rm -r bin
