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

#define PAGE_SIZE	4096

int num_threads = 4;
char *buf;
char *buf1;

void *pthread_transfer(void *arg)
{
	int pid = *(int *)arg;

	memcpy(buf + pid * PAGE_SIZE, buf1, PAGE_SIZE);

	pthread_exit(0);
}

int main(void)
{
	pthread_t *pthreads;
	int pids[16];
	int i;

	buf1 = malloc(PAGE_SIZE);
	memset(buf1, 'c', PAGE_SIZE);

	buf = malloc(PAGE_SIZE * 3);

	// Allocate threads
        pthreads = (pthread_t *)malloc(num_threads * sizeof(pthread_t));
        for (i = 0; i < num_threads; i++) {
                pids[i] = i;
                pthread_create(pthreads + i, NULL, pthread_transfer, (void *)(pids + i));
        }

	for (i = 0; i < num_threads; i++) {
		pthread_join(pthreads[i], NULL);
	}

	free(buf);
	free(buf1);
	free(pthreads);
	return 0;
}
