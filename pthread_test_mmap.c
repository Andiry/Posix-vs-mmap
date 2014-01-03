#define _GNU_SOURCE

#include<stdio.h>
#include<fcntl.h>
#include<time.h>
#include<string.h>
#include<malloc.h>
#include<stdlib.h>
#include<stdint.h>
#include<stdbool.h>
#include<sys/mman.h>

#include "memcpy.h"

#define END_SIZE	(4UL * 1024 * 1024) 

const int start_size = 512;
unsigned long long FILE_SIZE;
char **buf;
int num_threads;
int fd;
int read;
char *data;

//Doorbell. Each 64 bytes long to avoid cache contention.
volatile uint64_t doorbell[8 * 16];

#define START		0x1
#define FINISHED	0x2

void start_all_pthreads(void)
{
	int i;

	for (i = 0; i < num_threads; i++)
		doorbell[i * 8] = START;

}

bool all_pthreads_finished(void)
{
	int i;

	for (i = 0; i < num_threads; i++)
		if (doorbell[i * 8] != FINISHED)
			return false;

	return true;
}

inline bool pthread_can_start(int pid)
{
	return	(doorbell[pid * 8] == START);
}

inline void signal_pthread_finished(int pid)
{
	doorbell[pid * 8] = FINISHED;
}


void pthread_transfer(void *arg)
{
	int pid = *(int *)arg;
	size_t start_offset = FILE_SIZE / num_threads * pid;
	int size = start_size;
	unsigned long long count, offset;
	int i;
	char *data_begin;

//	printf("start pthread: %d\n", pid);
	while (size <= END_SIZE) {
		data_begin = data + start_offset;
		while (!pthread_can_start(pid))
			;

		count = FILE_SIZE / (num_threads * size);
		for (i = 0; i < count; i++) { 
			if (read)
				memcpy(buf[pid], data_begin, size);
			else 
				mmx2_memcpy(data_begin, buf[pid], size);
			data_begin += size;
		}

		signal_pthread_finished(pid);
		size <<= 1;
	}

	pthread_exit(0);
}

int main(int argc, char **argv)
{
	pthread_t *pthreads;
	int pids[16];
	int i, j;
	long long time;
	size_t len;
	char c = 'a';
	char unit;
	struct timespec start, end;
	int size;
	unsigned long long count;
	FILE *output;
	char fs_type[20];
	char quill_enabled[40];
	char file_size_num[20];
	char filename[60];
	unsigned long long offset;

	if (argc < 7) {
		printf("Usage: ./write_to_ram_multithread $FS $QUILL $fops $num_threads $FILE_SIZE $filename\n");
		return 0;
	}

	strcpy(fs_type, argv[1]);
	strcpy(quill_enabled, argv[2]);

	if (!strcmp(argv[3], "read"))
		read = 1;
	else if (!strcmp(argv[3], "write"))
		read = 0;
	else {
		printf("fops error!\n");
		return 0;
	}

	num_threads = atoi(argv[4]);
	if (num_threads <= 0 || num_threads > 16)
		num_threads = 1;

	strcpy(file_size_num, argv[5]);
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

	strcpy(filename, argv[6]);
	output = fopen(filename, "a");

	printf("# pthreads: %d\n", num_threads);
	buf = (char **)malloc(num_threads * sizeof(char *));

	for (i = 0; i < num_threads; i++) { 
		if (posix_memalign((void **)(buf + i), END_SIZE, END_SIZE)) { // up to 64MB
			printf("ERROR - POSIX NOMEM!\n");
			return 0;
		}
	}

	fd = open("/mnt/ramdisk/test1", O_CREAT | O_RDWR | O_DIRECT, 0640); 
	data = (char *)mmap(NULL, FILE_SIZE, PROT_WRITE, MAP_SHARED, fd, 0);

	//Warm up
	printf("warm up...\n");
//	count = FILE_SIZE / (size * num_threads);
	for (i = 0; i < num_threads; i++) {
		memcpy(buf[i], data, END_SIZE);
	}
	printf("warm up done.\n");

	//Allocate the threads
	pthreads = (pthread_t *)malloc(num_threads * sizeof(pthread_t));
	for (i = 0; i < num_threads; i++) {
		pids[i] = i;
		pthread_create(pthreads + i, NULL, pthread_transfer, (void *)(pids + i)); 
	}

	for (size = start_size; size <= END_SIZE; size <<= 1) {
		for (i = 0; i < num_threads; i++)
			memset(buf[i], c, size);
		c++;
		lseek(fd, 0, SEEK_SET);

		clock_gettime(CLOCK_MONOTONIC, &start);
		start_all_pthreads();

		while (!all_pthreads_finished())
			;
		clock_gettime(CLOCK_MONOTONIC, &end);

		time = (end.tv_sec - start.tv_sec) * 1e9 + (end.tv_nsec - start.tv_nsec);
		printf("%s: Size %d bytes,\t %lld times,\t %lld nanoseconds,\t Bandwidth %f MB/s.\n", argv[3], size, count, time, FILE_SIZE * 1024.0 / time);
		fprintf(output, "%s,%s,%s,%d,%d,%lld,%lld,%lld,%f\n", fs_type, quill_enabled, argv[3], num_threads, size, FILE_SIZE, count, time, FILE_SIZE * 1.0 / time);
	}

	fclose(output);
	close(fd);
	for (i = 0; i < num_threads; i++) {
		pthread_join(pthreads[i], NULL);
		free(buf[i]);
	}
	free(buf);
	free(pthreads);
	munmap(data, FILE_SIZE);
	return 0;
}