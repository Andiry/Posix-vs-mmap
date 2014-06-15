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

#define END_SIZE	(4UL * 1024 * 1024) 

const int start_size = 512;

int main(int argc, char ** argv)
{
	int fd, i;
	long long time, time1;
	unsigned long long FILE_SIZE;
	unsigned long long MMAP_UNIT;
	size_t len;
	char c = 'A';
	char unit;
	char *origin_data;
	char *data;
	struct timespec start, end;
	struct timeval begin, finish;
	struct timezone tz;
	void *buf1 = NULL;
	char *buf;
	int size, count;
	FILE *output;
	char fs_type[20];
	char file_size_num[20];
	char mmap_unit_num[20];
	char use_nvp[20];
	char filename[60];
	off_t offset;
	int mmap_times;

	if (argc < 6) {
		printf("Usage: ./mmap-test $FS $SCNEARIO $MMAP_UNIT $FILE_SIZE $filename\n");
		return 0;
	}

	strcpy(fs_type, argv[1]);
	strcpy(use_nvp, argv[2]);

	strcpy(mmap_unit_num, argv[3]);
	len = strlen(mmap_unit_num);
	unit = mmap_unit_num[len - 1];
	mmap_unit_num[len - 1] = '\0';
	MMAP_UNIT = atoll(mmap_unit_num);
	switch (unit) {
	case 'K':
	case 'k':
		MMAP_UNIT *= 1024;
		break;
	case 'M':
	case 'm':
		MMAP_UNIT *= 1048576;
		break;
	case 'G':
	case 'g':
		MMAP_UNIT *= 1073741824;
		break;
	default:
		printf("ERROR: MMAP_UNIT should be #K/M/G format.\n");
		return 0;
		break;
	}

	strcpy(file_size_num, argv[4]);
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

	strcpy(filename, argv[5]);
	output = fopen(filename, "a");

	if (posix_memalign(&buf1, END_SIZE, END_SIZE)) {
		printf("ERROR - POSIX NOMEM!\n");
		return 0;
	}

	buf = (char *)buf1;

	fd = open("/mnt/ramdisk1/test1", O_CREAT | O_RDWR, 0640); 
//	data = (char *)mmap(NULL, FILE_SIZE, PROT_WRITE, MAP_SHARED, fd, 0);
	data = (char *)mmap(NULL, 1073741824, PROT_WRITE, MAP_SHARED, fd, 0);
	origin_data = data;

	for (size = start_size; size <= END_SIZE; size <<= 1) {
//		size = atoi(argv[2]);
//		enable_ftrace = atoi(argv[3]);
		offset = 0;
		count = FILE_SIZE / size;
		data = origin_data;
		mmap_times = 0;

		clock_gettime(CLOCK_MONOTONIC, &start);
		for (i = 0; i < count; i++) {
			if (offset % MMAP_UNIT == 0) {
				mmap_times++;
				data = (char *)mmap(NULL, MMAP_UNIT, PROT_WRITE, MAP_SHARED, fd, 0);
			}
			memcpy(buf, data + (offset % MMAP_UNIT), size);
			offset += size;
		}
		msync(origin_data, FILE_SIZE, MS_ASYNC);
		clock_gettime(CLOCK_MONOTONIC, &end);

		time = (end.tv_sec - start.tv_sec) * 1e9 + (end.tv_nsec - start.tv_nsec);
		printf("mmap: %d times, Size %d bytes,\t %lld times,\t %lld nanoseconds,\t latency %lld nanoseconds, \t Bandwidth %f MB/s.\n", mmap_times, size, count, time, time / count, FILE_SIZE * 1024.0 / time);
//		fprintf(output, "%s,%s,%d,%lld,%d,%lld,%f\n", fs_type, use_nvp, size, FILE_SIZE, count, time, FILE_SIZE * 1.0 / time);
		fprintf(output, "%s,%s,%d,%lld,%lld,%lld,%f,%lld\n", fs_type, use_nvp, size, FILE_SIZE, count, time, FILE_SIZE * 1.0 / time, time / count);
	}

	fclose(output);
//	munmap(origin_data, FILE_SIZE);
//	munmap(origin_data, 1073741824);
	close(fd);
	free(buf1);
	return 0;
}
