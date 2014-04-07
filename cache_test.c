#define _GNU_SOURCE

#include<stdio.h>
#include<fcntl.h>
#include<time.h>
#include<string.h>
#include<malloc.h>
#include<stdlib.h>
#include<sys/mman.h>
#include<sys/time.h>

int main(int argc, char **argv)
{
	int fd, i;
	long long time, time1;
	unsigned long long FILE_SIZE;
	size_t len;
	off_t offset;
	char unit;
	struct timespec start, end;
	struct timespec begin, finish;
	struct timezone tz;
	int size;
	unsigned long long count;
	char *buf, *data;

	clock_gettime(CLOCK_MONOTONIC, &begin);
	for (i = 0; i < 1; i++) {
		getuid();
	}
	clock_gettime(CLOCK_MONOTONIC, &finish);

	buf = malloc(4096);
	memset(buf, 'c', 4096);
	fd = open("/mnt/ramdisk/test2", O_RDWR | O_CREAT, 0640);
	data = (char *)mmap(NULL, 4096, PROT_WRITE, MAP_SHARED, fd, 0);

//	write(fd, buf, 4096);
//	memcpy(data, buf, 4096);
	munmap(data, 4096);
	close(fd);

	time1 = (finish.tv_sec * 1e9 + finish.tv_nsec) - (begin.tv_sec * 1e9 + begin.tv_nsec);
	printf("getuid %lld ns, average %lld ns\n", time1, time1 / 1000000);

	return 0;
}
