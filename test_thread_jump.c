#define _GNU_SOURCE

#include<stdio.h>
#include<fcntl.h>
#include<time.h>
#include<string.h>
#include<malloc.h>
#include<stdlib.h>
#include<stdint.h>
#include<signal.h>
#include<setjmp.h>
#include<stdbool.h>
#include<pthread.h>
#include<sys/mman.h>
#include<sys/time.h>

#define PAGE_SIZE	4096

int num_threads = 4;
char *buf;
char *buf1;

static sigjmp_buf jumper;

void signal_handler(int sig)
{
	siglongjmp(jumper, 1);
}

void setup_signal_handler(void)
{
	struct sigaction act, oact;
	act.sa_handler = signal_handler;
	act.sa_flags = SA_NODEFER;

	sigaction(SIGSEGV, &act, &oact);
}

void *pthread_transfer(void *arg)
{
	int pid = *(int *)arg;
	int segfault;

	printf("Thread %d starts\n", pid);
	segfault = sigsetjmp(jumper, 0);

	if (segfault == 0) {
		memcpy(buf + pid * PAGE_SIZE, buf1, PAGE_SIZE);
	} else {
		printf("Thread %d triggers seg fault\n", pid);
	}

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

	setup_signal_handler();

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
