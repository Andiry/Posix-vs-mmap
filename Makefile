CC = gcc
CFLAGS = -O3 -Wall
CLIB = -lrt -lpthread -lnoposix

SRCS = $(wildcard *.c)
BUILD = $(patsubst %.c, %, $(SRCS))

all: $(BUILD)

.c:
	$(CC) $(CFLAGS) $< -o $@ $(CLIB)

clean:
	rm -rf $(BUILD)

