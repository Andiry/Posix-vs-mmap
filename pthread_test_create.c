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
#include<sys/time.h>


char *dir = "/mnt/ramdisk/file_";
int num_threads;
int num_files;

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


void* pthread_file_create(void *arg)
{
	struct timespec start, end;
	long long time;
	char filename[60];
	char command[120];
	int pid = *(int *)arg;
	int fd;
	int file_count;

//	printf("start pthread: %d\n", pid);
	while (!pthread_can_start(pid))
		;

	clock_gettime(CLOCK_MONOTONIC, &start);
	for (file_count = 0; file_count < num_files; file_count++) {
		sprintf(filename, "%s%d%d", dir, pid, file_count);
//		sprintf(command, "%s %s", "touch", filename);
//		system(command);
		fd = open(filename, O_CREAT | O_RDWR, 0640);

		close(fd);
	}
	clock_gettime(CLOCK_MONOTONIC, &end);

	time = (end.tv_sec - start.tv_sec) * 1e9 + (end.tv_nsec - start.tv_nsec);
	printf("Thread %d: %lld nanoseconds, each %lld ns\n", pid, time, time / num_files);

	signal_pthread_finished(pid);

	pthread_exit(0);
	return NULL;
}

int main(int argc, char **argv)
{
	pthread_t *pthreads;
	int pids[16];
	int i;
	long long time;
	struct timespec start, end;

	if (argc < 3) {
		printf("Usage: ./pthread_test $NUM_FILE $NUM_THREADS\n");
		return 0;
	}

	num_files = atoi(argv[1]);

	num_threads = atoi(argv[2]);
	if (num_threads <= 0 || num_threads > 16)
		num_threads = 1;

	printf("%d files, %d pthreads\n", num_files, num_threads);
	system("rm -rf /mnt/ramdisk/*");

	//Allocate the threads
	pthreads = (pthread_t *)malloc(num_threads * sizeof(pthread_t));
	for (i = 0; i < num_threads; i++) {
		pids[i] = i;
		pthread_create(pthreads + i, NULL, pthread_file_create, (void *)(pids + i));
	}

	clock_gettime(CLOCK_MONOTONIC, &start);
	start_all_pthreads();

	while (!all_pthreads_finished())
		;
	clock_gettime(CLOCK_MONOTONIC, &end);

	time = (end.tv_sec - start.tv_sec) * 1e9 + (end.tv_nsec - start.tv_nsec);
	printf("%lld nanoseconds, each %lld ns\n", time, time / (num_files * num_threads));

	for (i = 0; i < num_threads; i++) {
		pthread_join(pthreads[i], NULL);
	}

	free(pthreads);
	return 0;
}
