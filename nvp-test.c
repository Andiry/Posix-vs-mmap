#define _GNU_SOURCE

#include<stdio.h>
#include<fcntl.h>
#include<time.h>
#include<string.h>
#include<malloc.h>
#include<stdlib.h>
#include<pthread.h>
#include<sys/mman.h>
#include<sys/time.h>

#define END_SIZE	(64UL * 1024 * 1024) 

const int start_size = 512;

int main(int argc, char **argv)
{
	int fd, i;
	long long time, time1;
	unsigned long long FILE_SIZE;
	size_t len;
	off_t offset;
	char c = 'a';
	char unit;
	struct timespec start, end;
	struct timeval begin, finish;
	struct timezone tz;
	int size;
	unsigned long long count;
	void *buf1 = NULL;
	char *buf;
	FILE *output;
	pthread_rwlock_t lock;

	size = 4096;

	if (posix_memalign(&buf1, END_SIZE, END_SIZE)) { // up to 64MB
		printf("ERROR - POSIX NOMEM!\n");
		return 0;
	}

	buf = (char *)buf1;
	fd = open("/mnt/ramdisk/test1", O_CREAT | O_RDWR | O_DIRECT, 0640); 
//	fd = open("/dev/null", O_WRONLY, 0640); 
//	fd = open("/dev/zero", O_RDONLY, 0640); 
	printf("fd: %d\n", fd);
	printf("plock size: %d %d\n", sizeof(lock), sizeof(int));
	memset(buf, c, END_SIZE);

	count = 262144;
	for (i = 0; i < count; i++) {
		if (pread(fd, buf, size, offset) != size)
			printf("ERROR!\n");
		offset += size;
	}
	offset = 0;

	gettimeofday(&begin, &tz);
	clock_gettime(CLOCK_MONOTONIC, &start);
	for (i = 0; i < count; i++) {
		if (pread(fd, buf, size, offset) != size)
			printf("ERROR!\n");
		offset += size;
	}
	printf("DONE\n");

	clock_gettime(CLOCK_MONOTONIC, &end);
	gettimeofday(&finish, &tz);

	time = (end.tv_sec - start.tv_sec) * 1e9 + (end.tv_nsec - start.tv_nsec);
	time1 = (finish.tv_sec - begin.tv_sec) * 1e6 + (finish.tv_usec - begin.tv_usec);
	printf("Read: Size %d bytes,\t %lld times,\t %lld nanoseconds,\t latency %lld nanoseconds, \t Bandwidth %f MB/s.\n", size, count, time, time / count, size * count * 1024.0 / time);
	printf("Read process %lld microseconds\n", time1);

	close(fd);
	free(buf1);
	return 0;
}
