all: write_to_ram mmap_to_ram

clean:
	rm -rf write_to_ram *.o mmap_to_ram

write_to_ram: write_to_ram.c
	gcc -O3 write_to_ram.c -o write_to_ram -lrt

mmap_to_ram: mmap_to_ram.c
	gcc -O3 mmap_to_ram.c -o mmap_to_ram -lrt

