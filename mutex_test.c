#define _GNU_SOURCE
#include<pthread.h>
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<wait.h>

pthread_mutex_t mutex;

void *another(void *arg)
{
	printf("In child thread, taking mutex %p\n", &mutex);
	pthread_mutex_lock(&mutex);
	sleep(5);
	pthread_mutex_unlock(&mutex);
	printf("Return from child thread\n");
	return NULL;
}

int main(void)
{
	int pid;
	pthread_t id;
	pthread_mutex_init(&mutex, NULL);

	pthread_create(&id, NULL, another, NULL);

	sleep(1);
	pid = fork();
	if (pid == 0) {
		printf("I am in the child, want to get lock %p\n", &mutex);
		pthread_mutex_lock(&mutex);
		printf("Oops\n");
		pthread_mutex_unlock(&mutex);
	} else
		wait(NULL);

	pthread_join(id, NULL);
	pthread_mutex_destroy(&mutex);
	return 0;
}
