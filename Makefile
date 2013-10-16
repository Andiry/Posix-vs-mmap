all: write_to_ram

clean:
	rm -rf write_to_ram *.o

write_to_ram: write_to_ram.o -lrt

