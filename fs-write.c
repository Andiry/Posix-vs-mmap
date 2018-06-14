#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

#define FILESIZE (1024 * 1024 * 1024)

int main(int argc, char *argv[])
{
    int fd, i;
    int result;
    char *map;

	if (argc != 2) {
		printf("Please specify filename\n");
		exit(EXIT_FAILURE);
	}

    fd = open(argv[1], O_RDWR | O_CREAT | O_TRUNC, (mode_t)0600);
    if (fd == -1) {
		perror("Error opening file for writing");
		exit(EXIT_FAILURE);
    }

    result = lseek(fd, FILESIZE-1, SEEK_SET);
    if (result == -1) {
		close(fd);
		perror("Error calling lseek() to 'stretch' the file");
		exit(EXIT_FAILURE);
    }
    
    result = write(fd, "", 1);
    if (result != 1) {
		close(fd);
		perror("Error writing last byte of the file");
		exit(EXIT_FAILURE);
    }

    map = mmap(0, FILESIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (map == MAP_FAILED) {
		close(fd);
		perror("Error mmapping the file");
		exit(EXIT_FAILURE);
    }

	/* trigger #PF */
    for (i = 0; i < FILESIZE / (4096 * 512); i++)
	map[4096 * 512 * i] = 'd'; 

    if (munmap(map, FILESIZE) == -1)
		perror("Error un-mmapping the file");
    close(fd);
    return 0;
}
