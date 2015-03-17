.PHONY: clean

CC=gcc
CXX=g++
CFLAGS=-D_XOPEN_SOURCE=600 -D_BSD_SOURCE -std=gnu99 -pedantic -Wall -W -Wundef -Wendif-labels -Wshadow -Wpointer-arith -Wbad-function-cast -Wcast-align -Wwrite-strings -Wstrict-prototypes -Wmissing-prototypes -Wnested-externs -Winline -Wdisabled-optimization -fstrict-aliasing -O2 -pipe -Wno-parentheses

all: sum

sum: sum.c
	$(CC) $(CFLAGS) -o sum sum.c

clean:
	rm -f sum

