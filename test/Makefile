CFLAGS += -O2 -Wall -Wextra -I../include

all: test

run: test
	./test

clean:
	find ./* -maxdepth 0 -not -name '*.c' -not -name 'Makefile' -exec rm -rf {} \;
