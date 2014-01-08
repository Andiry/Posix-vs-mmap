#define _GNU_SOURCE
#define _STDC_FORMAT_MACROS

#include<stdio.h>
#include<fcntl.h>
#include<time.h>
#include<string.h>
#include<malloc.h>
#include<stdlib.h>
#include<stdint.h>
#include<stdbool.h>
#include<inttypes.h>
#include<sys/mman.h>
#include "Util/FastRand.hpp"

#define END_SIZE	(4UL * 1024 * 1024) 

const int start_size = 512;
unsigned long long FILE_SIZE;
char **buf;
int num_threads;
int *fd;
uint64_t *random_array;

ssize_t read(int, void *, size_t);
ssize_t write(int, void *, size_t);
ssize_t pread(int, void *, size_t, off_t);
ssize_t pwrite(int, void *, size_t, off_t);
ssize_t (*fops)(int, void *, size_t, ...);

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
	unsigned long long start_offset = FILE_SIZE / num_threads * pid;
	int size = start_size;
	unsigned long long count, offset;
	int i, error;

//	printf("start pthread: %d\n", pid);
	while (size <= END_SIZE) {
		while (!pthread_can_start(pid))
			;

		error = 0;
		offset = start_offset;
		lseek(fd[pid], offset, SEEK_SET);
		count = FILE_SIZE / (num_threads * size);
		for (i = 0; i < count; i++) { 
			if (read(fd[pid], buf[pid], size) != size)
				printf("ERROR\n");
//			if (fops(fd[pid], buf[pid], size, offset) != size)
			// check for error
			if (*(uint64_t *)(buf[pid]) != *(random_array + offset / size)) {
				error++;
//				printf("%" PRIu64 " %"PRIu64"\n", *(uint64_t *)(buf + pid), *(random_array + offset /size));
			}
			offset += size;
		}

		if (error)
			printf("pid %d, size %d, count %d, error %d\n", pid, size, count, error);

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
	uint64_t seed;
	int seq_count, k;

	if (argc < 7) {
		printf("Usage: ./integrity_test $FS $QUILL $seed $num_threads $FILE_SIZE $filename\n");
		return 0;
	}

	strcpy(fs_type, argv[1]);
	strcpy(quill_enabled, argv[2]);

	seed = atoi(argv[3]);

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

	printf("# fds: ");
	fd = malloc(num_threads * sizeof(int));
	for (i = 0; i < num_threads; i++) {
		fd[i] = open("/mnt/ramdisk/test1", O_CREAT | O_RDWR | O_DIRECT, 0640); 
		printf("%d ", fd[i]);
	}
	printf("\n");

	//Fill up random array
	random_array = malloc(sizeof(uint64_t) * (FILE_SIZE / start_size));
	count = FILE_SIZE / start_size;
	for (i = 0; i < count; i++) {
		random_array[i] = seed;
//		printf("%" PRIu64" " , random_array[i]);
		RandLFSR(&seed);
	}

	//Allocate the threads
	pthreads = (pthread_t *)malloc(num_threads * sizeof(pthread_t));
	for (i = 0; i < num_threads; i++) {
		pids[i] = i;
		pthread_create(pthreads + i, NULL, pthread_transfer, (void *)(pids + i)); 
	}

	for (size = start_size; size <= END_SIZE; size <<= 1) {
		for (i = 0; i < num_threads; i++)
			lseek(fd[i], 0, SEEK_SET);

		//Write to file
		printf("warm up...\n");
		seq_count = size / sizeof(uint64_t);
		count = FILE_SIZE / (num_threads * size);
		for (i = 0; i < num_threads; i++) {
			offset = FILE_SIZE / num_threads * i;
			lseek(fd[i], offset, SEEK_SET);
			for (j = 0; j < count; j++) {
//			printf("write %" PRIu64" " , random_array[i*offset+j]);
				for (k = 0; k < seq_count; k++) 
					write(fd[i], random_array + offset / size + j, sizeof(uint64_t));
			}
		}

		printf("start threads...\n");
		clock_gettime(CLOCK_MONOTONIC, &start);
		start_all_pthreads();

		while (!all_pthreads_finished())
			;
		clock_gettime(CLOCK_MONOTONIC, &end);

		time = (end.tv_sec - start.tv_sec) * 1e9 + (end.tv_nsec - start.tv_nsec);
		printf("%s: Size %d bytes,\t %lld times,\t %lld nanoseconds,\t Bandwidth %f MB/s.\n", argv[3], size, count, time, FILE_SIZE * 1024.0 / time);
//		fprintf(output, "%s,%s,%s,%d,%d,%lld,%lld,%lld,%f\n", fs_type, quill_enabled, argv[3], num_threads, size, FILE_SIZE, count, time, FILE_SIZE * 1.0 / time);
	}

	fclose(output);
	for (i = 0; i < num_threads; i++) {
		pthread_join(pthreads[i], NULL);
		free(buf[i]);
		close(fd[i]);
	}
	free(buf);
	free(fd);
	free(pthreads);
	free(random_array);
	return 0;
}
