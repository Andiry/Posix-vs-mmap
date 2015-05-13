#define _GNU_SOURCE

#include<stdio.h>
#include<fcntl.h>
#include<time.h>
#include<string.h>
#include<malloc.h>
#include<stdlib.h>
#include<stdint.h>
#include<stdbool.h>
#include<pthread.h>
#include<sys/mman.h>

#include "memcpy.h"

#define END_SIZE	(4UL * 1024 * 1024) 

const int start_size = 2048;
unsigned long long FILE_SIZE;
char *buf;
int num_threads;
int fd;
int read;
char *data;
volatile char *data1;
volatile int waiting_threads;
volatile int finished_threads;
volatile int size;
pthread_cond_t	ready = PTHREAD_COND_INITIALIZER;
pthread_cond_t	finish = PTHREAD_COND_INITIALIZER;
pthread_mutex_t	lock = PTHREAD_MUTEX_INITIALIZER;

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


void *pthread_transfer(void *arg)
{
	int pid = *(int *)arg;
	unsigned long long count;
	int unit = size / num_threads;
	size_t offset = unit * pid;
	int i;
	char *data_begin;

//	printf("start pthread: %d\n", pid);

	while(1) {
		unit = size / num_threads;
		pthread_mutex_lock(&lock);
		while (waiting_threads == 0)
			pthread_cond_wait(&ready, &lock);
		waiting_threads &= ~(1 << pid);
		pthread_mutex_unlock(&lock);

		data_begin = data1 + offset;
//		printf("start pthread: %d, %p, %d, %d\n", pid, data_begin, size, unit);
		if (read)
			memcpy(buf + offset, data_begin, unit);
		else 
			mmx2_memcpy(data_begin, buf + offset, unit);

//		printf("Finish pthread: %d\n", pid);
		pthread_mutex_lock(&lock);
		finished_threads |= 1 << pid;
//		printf("Finished pthreads: %d\n", finished_threads);
		pthread_mutex_unlock(&lock);
		if (finished_threads == (1 << num_threads) - 1)
			pthread_cond_signal(&finish);
	}

	pthread_exit(0);
	return NULL;
}

int main(int argc, char **argv)
{
	pthread_t *pthreads;
	int pids[16];
	int i;
	long long time;
	size_t len;
	char c = 'a';
	char unit;
	struct timespec start, end;
	unsigned long long count;
	FILE *output;
	char fs_type[20];
	char quill_enabled[40];
	char file_size_num[20];
	char filename[60];

	if (argc < 7) {
		printf("Usage: ./pthread_test_mmap $FS $QUILL $fops $num_threads $FILE_SIZE $filename\n");
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

	if (posix_memalign((void *)&buf, END_SIZE, END_SIZE)) // up to 64MB
		return 0;

	fd = open("/mnt/ramdisk/test1", O_CREAT | O_RDWR, 0640); 
	data = (char *)mmap(NULL, FILE_SIZE, PROT_WRITE, MAP_SHARED | MAP_POPULATE, fd, 0);
	data1 = data;

	//Warm up
	printf("warm up...\n");
//	count = FILE_SIZE / (size * num_threads);
//	for (i = 0; i < num_threads; i++) {
//		memcpy(buf[i], data, END_SIZE);
//	}
	printf("warm up done.\n");

	//Allocate the threads
	pthreads = (pthread_t *)malloc(num_threads * sizeof(pthread_t));
	for (i = 0; i < num_threads; i++) {
		pids[i] = i;
		pthread_create(pthreads + i, NULL, pthread_transfer, (void *)(pids + i)); 
	}

	for (size = start_size; size <= END_SIZE; size <<= 1) {
		count = FILE_SIZE / size;
		lseek(fd, 0, SEEK_SET);
		data1 = data;

		clock_gettime(CLOCK_MONOTONIC, &start);
		for (i = 0; i < count; i++) {
//			printf("%d\n", i);
			pthread_mutex_lock(&lock);
//			printf("Set threads %d\n", i);
			waiting_threads = (1 << num_threads) - 1;
			finished_threads = 0;
			pthread_mutex_unlock(&lock);
			pthread_cond_broadcast(&ready);

			pthread_mutex_lock(&lock);
//			printf("Waiting %d\n", i);
			while (finished_threads != (1 << num_threads) - 1) {
//				printf("Checking: %d\n", finished_threads);
				pthread_cond_wait(&finish, &lock);
			}
//			printf("Finished %d\n", i);
			pthread_mutex_unlock(&lock);
			data1 += size;
		}

		clock_gettime(CLOCK_MONOTONIC, &end);

		time = (end.tv_sec - start.tv_sec) * 1e9 + (end.tv_nsec - start.tv_nsec);
		printf("%s: Size %d bytes,\t %lld times,\t %lld nanoseconds,\t Bandwidth %f MB/s, latenct %lld nanoseconds.\n", argv[3], size, count, time, FILE_SIZE * 1024.0 / time, time / count);
		fprintf(output, "%s,%s,%s,%d,%d,%lld,%lld,%lld,%f\n", fs_type, quill_enabled, argv[3], num_threads, size, FILE_SIZE, count, time, FILE_SIZE * 1.0 / time);
	}

	fclose(output);
	close(fd);
	for (i = 0; i < num_threads; i++) {
		pthread_join(pthreads[i], NULL);
	}
	free(buf);
	free(pthreads);
	munmap(data, FILE_SIZE);
	return 0;
}
