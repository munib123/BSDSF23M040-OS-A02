CC = gcc
CFLAGS = -Wall -g

all: bin/ls

bin/ls: src/ls-v1.4.0.c
	$(CC) $(CFLAGS) -o bin/ls src/ls-v1.4.0.c

clean:
	rm -f bin/ls obj/*