all: write_to_ram mmap_to_ram

clean:
	rm -rf write_to_ram *.o mmap_to_ram

write_to_ram: write_to_ram.o -lrt
mmap_to_ram: mmap_to_ram.o -lrt 

