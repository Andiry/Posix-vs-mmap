#include<stdio.h>
#include<stdlib.h>
#include<malloc.h>
#include<unistd.h>
#include<fcntl.h>
#include<time.h>
#include<sys/mman.h>
#include<string.h>
#include<pthread.h>

#define END_SIZE	(1 * 1024 * 1024) 
#define FILE_SIZE	(4 * 1024 * 1024) 

char *buf;
char *data;

const int start_size = 512;

#if 0
void* mmap_transfer(void *arg)
{
	int id = (int)((*(int *)arg) & 0xff);
	int size = (int)((*(int *)arg) >> 8);
	int offset = id * SIZE / num_threads;
	int count, i;
	char *start_data = data + offset;
	char *start_buf = buf + offset;

//	printf("%d %d\n", id, start_size);
	count = SIZE / (size * num_threads);
	for (i = 0; i < count; i++) {
		memcpy(start_data, start_buf, size);
		start_data += size;
		start_buf += size;
	}
}
#endif

int main(int argc, char ** argv)
{
	int fd, i;
	unsigned long time;
	char c = 'A';
	char *origin_data;
	struct timespec start, end;
	void *buf1 = NULL;
	int size, count;
	pthread_t *pid;
	int *pdata;
	void *thread_ret;

	posix_memalign(&buf1, FILE_SIZE, FILE_SIZE);

	buf = (char *)buf1;

	fd = open("/mnt/ramdisk/test1", O_CREAT | O_RDWR); 
	data = (char *)mmap(NULL, FILE_SIZE, PROT_WRITE, MAP_SHARED, fd, 0);
	origin_data = data;

	for (size = start_size; size <= END_SIZE; size <<= 1) {
		buf = (char *)buf1;
		memset(buf, c, size);
		c++;
		data = origin_data;

		clock_gettime(CLOCK_MONOTONIC, &start);
		count = FILE_SIZE / size;
		for (i = 0; i < count; i++) {
			memcpy(data, buf, size);
			data += size;
		}

		msync(origin_data, FILE_SIZE, MS_ASYNC);
		clock_gettime(CLOCK_MONOTONIC, &end);

		time = (end.tv_sec - start.tv_sec) * 1e9 + (end.tv_nsec - start.tv_nsec);
		printf("Mmap: size %d bytes, %d times,\t %ld nanoseconds,\t Bandwidth %f MB/s.\n", size, count, time, FILE_SIZE * 1024.0 / time);
	}

	munmap(origin_data, FILE_SIZE);
	close(fd);
	free(buf1);
	return 0;
}
