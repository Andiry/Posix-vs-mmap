#define _GNU_SOURCE

#include<stdio.h>
#include<stdlib.h>
#include<malloc.h>
#include<unistd.h>
#include<fcntl.h>
#include<time.h>
#include<sys/mman.h>
#include<sys/time.h>
#include<string.h>
#include<pthread.h>

#include "memcpy.h"

#define END_SIZE	(4UL * 1024) 

const int start_size = 512;

int main(int argc, char ** argv)
{
	int fd;
	long long time;
	unsigned long long FILE_SIZE;
	size_t len;
	char unit;
	char *data;
	struct timespec start, end;
	void *buf1 = NULL;
	int i;
	int count;
	char file_size_num[20];

	if (argc < 2) {
		printf("Usage: ./munmap-test $FILE_SIZE\n");
		return 0;
	}

	strcpy(file_size_num, argv[1]);
	len = strlen(file_size_num);
	unit = file_size_num[len - 1];
	file_size_num[len - 1] = '\0';
	FILE_SIZE = atoll(file_size_num);
	switch (unit) {
	case 'K':
	case 'k':
		FILE_SIZE *= 1024;
		break;
	case 'M':
	case 'm':
		FILE_SIZE *= 1048576;
		break;
	case 'G':
	case 'g':
		FILE_SIZE *= 1073741824;
		break;
	default:
		printf("ERROR: FILE_SIZE should be #K/M/G format.\n");
		return 0;
		break;
	}

	if (FILE_SIZE < END_SIZE)
		FILE_SIZE = END_SIZE;
	if (FILE_SIZE > 2147483648) // RAM disk size
		FILE_SIZE = 2147483648;

	if (posix_memalign(&buf1, END_SIZE, END_SIZE)) {
		printf("ERROR - POSIX NOMEM!\n");
		return 0;
	}

	memset(buf1, 'a', END_SIZE);
	count = FILE_SIZE / END_SIZE;

	fd = open("/mnt/ramdisk/test1", O_CREAT | O_RDWR, 0640);
	for (i = 0; i < count; i++)
		write(fd, buf1, END_SIZE);

	clock_gettime(CLOCK_MONOTONIC, &start);
	data = (char *)mmap(NULL, END_SIZE, PROT_WRITE, MAP_SHARED | MAP_POPULATE, fd, 0);
	memset(data, 'b', END_SIZE);
	clock_gettime(CLOCK_MONOTONIC, &end);
//	munmap(data, END_SIZE);

	clock_gettime(CLOCK_MONOTONIC, &start);
	data = (char *)mmap(NULL, END_SIZE, PROT_WRITE, MAP_SHARED | MAP_POPULATE, fd, 8192);
	memset(data, 'c', END_SIZE);
	clock_gettime(CLOCK_MONOTONIC, &end);
//	munmap(data, END_SIZE);

	time = (end.tv_sec - start.tv_sec) * 1e9 + (end.tv_nsec - start.tv_nsec);
	printf("mmap: Size %lld bytes,\t %d pages,\t %lld nanoseconds,\t %lld nanoseconds per page\n",
		FILE_SIZE, count, time, time / count);

	clock_gettime(CLOCK_MONOTONIC, &start);
	msync(data, END_SIZE, MS_SYNC);
	clock_gettime(CLOCK_MONOTONIC, &end);

	time = (end.tv_sec - start.tv_sec) * 1e9 + (end.tv_nsec - start.tv_nsec);
	printf("msync: Size %lld bytes,\t %d pages,\t %lld nanoseconds,\t %lld nanoseconds per page\n",
		FILE_SIZE, count, time, time / count);

	munmap(data, END_SIZE);
	close(fd);
	free(buf1);
	return 0;
}
