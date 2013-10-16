#include<stdio.h>
#include<fcntl.h>
#include<time.h>
#include<sys/mman.h>
#include<string.h>

#define SIZE	(4 * 1024 * 1024) 

int main(void)
{
	int fd, i;
	unsigned long time;
	char c = 'b';
	char *data = NULL, *origin_data;
	struct timespec start, end;

	fd = open("/mnt/ramdisk/test2", O_CREAT | O_RDWR); 
	data = (char *)mmap(NULL, SIZE, PROT_WRITE, MAP_SHARED, fd, 0);
	origin_data = data;

	clock_gettime(CLOCK_MONOTONIC, &start);
	for (i = 0; i < SIZE; i++)
		memcpy(data++, &c, 1);

	msync(origin_data, SIZE, MS_SYNC);
	clock_gettime(CLOCK_MONOTONIC, &end);
	time = (end.tv_sec - start.tv_sec) * 1e9 + (end.tv_nsec - start.tv_nsec);
	printf("Mmap: Used %ld nanoseconds.\n", time);
	munmap(origin_data, SIZE);
	close(fd);
	return 0;
}
