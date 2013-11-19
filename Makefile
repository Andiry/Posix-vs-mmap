all: write_to_ram mmap_to_ram write_to_ram_warm

clean:
	rm -rf write_to_ram *.o mmap_to_ram write_to_ram_warm

write_to_ram: write_to_ram.c
	gcc -O3 write_to_ram.c -o write_to_ram -lrt

write_to_ram_warm: write_to_ram_warm.c
	gcc -O3 write_to_ram_warm.c -o write_to_ram_warm -lrt

mmap_to_ram: mmap_to_ram.c
	gcc -O3 mmap_to_ram.c -o mmap_to_ram -lrt

