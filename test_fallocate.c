#define _GNU_SOURCE

#include<stdio.h>
#include<unistd.h>
#include<fcntl.h>
#include<time.h>
#include<string.h>
#include<malloc.h>
#include<stdlib.h>
#include<stdint.h>
#include<stdbool.h>
#include<pthread.h>
#include<sys/mman.h>
#include<sys/time.h>

int main(void)
{
	int fd;
	int ret;
	int c = 32;
	size_t size;
	struct timespec start, end;
	unsigned long time;

	fd = open("/mnt/ramdisk/test2", O_CREAT | O_RDWR, 0640);

	size = write(fd, &c, 4);
	printf("%d %u\n", fd, size);

	clock_gettime(CLOCK_MONOTONIC, &start);
	ret = fallocate(fd, 0, 0, 1048576);
	clock_gettime(CLOCK_MONOTONIC, &end);

	time = (end.tv_sec - start.tv_sec) * 1e9 + end.tv_nsec - start.tv_nsec;

	printf("%d, %lu nanoseconds\n", ret, time);
	close(fd);
	return ret;
}
