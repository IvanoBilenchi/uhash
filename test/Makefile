CFLAGS += -O2 -Wall -Wextra -I../include

all: bench test

bench:

run: bench test
	./test && ./bench

clean:
	find ./* -maxdepth 0 -not -name '*.c' -not -name 'Makefile' -exec rm -rf {} \;
