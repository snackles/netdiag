CC ?= gcc
CFLAGS = -Wall -Wextra -std=gnu11 -O2

TARGET = netdiag
SOURCES = src/main.c src/utils.c src/commands.c
HEADERS = include/utils.h include/commands.h

all: $(TARGET)

$(TARGET): $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all run clean
