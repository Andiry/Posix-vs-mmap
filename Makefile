CC = gcc
CFLAGS = -O3
CLIB = -lrt -lpthread

SRCS = $(wildcard *.c)
BUILD = $(patsubst %.c, %, $(SRCS))

all: $(BUILD) multithread

.c:
	$(CC) $(CFLAGS) $< -o $@ $(CLIB)

multithread: multithread.o
	g++ $(CFLAGS) $^ -o $@ $(CLIB)

clean:
	rm -rf write_to_ram *.o mmap_to_ram write_to_ram_warm nvp-test

write_to_ram: write_to_ram.c
	gcc -O3 write_to_ram.c -o write_to_ram -lrt

write_to_ram_warm: write_to_ram_warm.c
	gcc -O3 write_to_ram_warm.c -o write_to_ram_warm -lrt

mmap_to_ram: mmap_to_ram.c
	gcc -O3 mmap_to_ram.c -o mmap_to_ram -lrt

