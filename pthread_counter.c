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

int shared_counter;
volatile int atomic_counter;
int num_threads;
int num;

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
	struct timespec start, end, temp;
	long long time;
	int pid = *(int *)arg;
	int i;

//	printf("start pthread: %d\n", pid);
	while (!pthread_can_start(pid))
		;

	clock_gettime(CLOCK_MONOTONIC, &start);
	for (i = 0; i < num; i++) {
//		shared_counter++;
//		__sync_fetch_and_add(&atomic_counter, 1);
		clock_gettime(CLOCK_MONOTONIC, &temp);
	}
	clock_gettime(CLOCK_MONOTONIC, &end);

	time = (end.tv_sec - start.tv_sec) * 1e9 + (end.tv_nsec - start.tv_nsec);
	printf("Thread %d: %lld nanoseconds, each %lld ns\n", pid, time, time / num);

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
		printf("Usage: ./pthread_counter $NUM $NUM_THREADS\n");
		return 0;
	}

	num = atoi(argv[1]);

	num_threads = atoi(argv[2]);
	if (num_threads <= 0 || num_threads > 16)
		num_threads = 1;

	printf("%d, %d pthreads\n", num, num_threads);

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
	printf("%lld nanoseconds, each %lld ns\n", time, time / (num * num_threads));
	printf("Shared counter %d\n", shared_counter);
	printf("Atomic counter %d\n", atomic_counter);

	for (i = 0; i < num_threads; i++) {
		pthread_join(pthreads[i], NULL);
	}

	free(pthreads);
	return 0;
}
