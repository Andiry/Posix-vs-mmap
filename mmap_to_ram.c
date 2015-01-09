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

//#define END_SIZE	(64UL * 1024 * 1024) 
#define END_SIZE	4096 

const int start_size = 4096;

int main(int argc, char ** argv)
{
	int fd, i;
	long long time, time1;
	unsigned long long FILE_SIZE;
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
	char use_nvp[20];
	char filename[60];
	int enable_ftrace;

	if (argc < 6) {
		printf("Usage: ./mmap_to_ram $FS $SCNEARIO $FTRACE $FILE_SIZE $filename\n");
		return 0;
	}

	strcpy(fs_type, argv[1]);
	strcpy(use_nvp, argv[2]);

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

//	fd = open("/mnt/ramdisk/test1", O_CREAT | O_RDWR | O_DIRECT, 0640); 
	fd = open("/mnt/ramdisk/test1", O_CREAT | O_RDWR, 0640); 

	gettimeofday(&begin, &tz);
//	data = (char *)mmap(NULL, FILE_SIZE, PROT_WRITE, MAP_SHARED, fd, 0);
	data = (char *)mmap(NULL, 1073741824, PROT_WRITE, MAP_SHARED | MAP_POPULATE, fd, 0);
//	data = (char *)mmap(NULL, 1073741824, PROT_WRITE, MAP_SHARED, fd, 0);
	origin_data = data;
	gettimeofday(&finish, &tz);

	enable_ftrace = atoi(argv[3]);
	for (size = start_size; size <= END_SIZE; size <<= 1) {
//		size = atoi(argv[2]);
//		enable_ftrace = atoi(argv[3]);
		memset(buf, c, size);
		c++;
		data = origin_data;
#if 0
		count = 1073741824 / size;
		// Warm the cache with 1GB write
		gettimeofday(&begin, &tz);
//		clock_gettime(CLOCK_MONOTONIC_RAW, &start);
		clock_gettime(CLOCK_MONOTONIC, &start);
		for (i = 0; i < count; i++) {
			mmx2_memcpy(data, buf, size);
			data += size;
		}

		msync(origin_data, 1073741824, MS_ASYNC);
		clock_gettime(CLOCK_MONOTONIC, &end);
		gettimeofday(&finish, &tz);
		time = (end.tv_sec - start.tv_sec) * 1e9 + (end.tv_nsec - start.tv_nsec);
		time1 = (finish.tv_sec - begin.tv_sec) * 1e6 + (finish.tv_usec - begin.tv_usec);
		printf("mmap warm: Size %d bytes,\t %lld times,\t %lld nanoseconds,\t latency %lld nanoseconds, \t Bandwidth %f MB/s.\n", size, count, time, time / count, FILE_SIZE * 1024.0 / time);
		printf("mmap warm cache process %lld microseconds\n", time1);
#endif
		count = FILE_SIZE / size;

		if (enable_ftrace)
			system("echo 1 > /sys/kernel/debug/tracing/tracing_on");
	
		clock_gettime(CLOCK_MONOTONIC, &start);
		for (i = 0; i < count; i++) {
//			mmx2_memcpy(data, buf, size);
			memcpy(buf, data, size);
			data += size;
		}
//		msync(origin_data, FILE_SIZE, MS_ASYNC);
		clock_gettime(CLOCK_MONOTONIC, &end);

		if (enable_ftrace)
			system("echo 0 > /sys/kernel/debug/tracing/tracing_on");
		time = (end.tv_sec - start.tv_sec) * 1e9 + (end.tv_nsec - start.tv_nsec);
		printf("mmap: Size %d bytes,\t %lld times,\t %lld nanoseconds,\t latency %lld nanoseconds, \t Bandwidth %f MB/s.\n", size, count, time, time / count, FILE_SIZE * 1024.0 / time);
		time1 = (finish.tv_sec - begin.tv_sec) * 1e6 + (finish.tv_usec - begin.tv_usec);
		printf("mmap process %lld microseconds\n", time1);
//		fprintf(output, "%s,%s,%d,%lld,%d,%lld,%f\n", fs_type, use_nvp, size, FILE_SIZE, count, time, FILE_SIZE * 1.0 / time);
		fprintf(output, "%s,%s,%d,%lld,%lld,%lld,%f,%lld\n", fs_type, use_nvp, size, FILE_SIZE, count, time, FILE_SIZE * 1.0 / time, time / count);
	}

	fclose(output);
//	munmap(origin_data, FILE_SIZE);
	munmap(origin_data, 1073741824);
	close(fd);
	free(buf1);
	return 0;
}
