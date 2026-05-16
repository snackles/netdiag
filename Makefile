CC ?= gcc
CFLAGS = -Wall -Wextra -std=c11 -O2
LDFLAGS =

all: netdiag

netdiag: src/main.c
	$(CC) $(CFLAGS) src/main.c -o netdiag $(LDFLAGS)

clean:
	rm -f netdiag
