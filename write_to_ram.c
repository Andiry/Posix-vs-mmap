#include<stdio.h>
#include<fcntl.h>
#include<time.h>

#define SIZE	(4 * 1024 * 1024) 

int main(void)
{
	int fd, i;
	unsigned long time;
	char c = 'x';
	struct timespec start, end;

	fd = open("/mnt/ramdisk/test1", O_CREAT | O_RDWR); 

	clock_gettime(CLOCK_MONOTONIC, &start);
	for (i = 0; i < SIZE; i++)
		write(fd, &c, 1);

	clock_gettime(CLOCK_MONOTONIC, &end);
	time = (end.tv_sec - start.tv_sec) * 1e9 + (end.tv_nsec - start.tv_nsec);
	printf("Used %ld nanoseconds.\n", time);
	close(fd);
	return 0;
}
