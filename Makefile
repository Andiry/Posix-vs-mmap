CC = gcc
CFLAGS = -O3
CLIB = -lrt -lpthread

SRCS = $(wildcard *.c)
BUILD = $(patsubst %.c, %, $(SRCS))

all: $(BUILD) multithread

.c:
	$(CC) $(CFLAGS) $< -o $@ $(CLIB)

clean:
	rm -rf $(BUILD)

