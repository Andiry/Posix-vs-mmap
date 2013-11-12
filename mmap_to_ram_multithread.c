#define _GNU_SOURCE

#include<stdio.h>
#include<stdlib.h>
#include<malloc.h>
#include<unistd.h>
#include<fcntl.h>
#include<time.h>
#include<sys/mman.h>
#include<string.h>
#include<pthread.h>

#define END_SIZE	(64UL * 1024 * 1024) 

const int start_size = 512;

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

int main(int argc, char ** argv)
{
	int fd, i;
	long long time;
	unsigned long long FILE_SIZE;
	size_t len;
	char unit
	char c = 'A';
	char *origin_data;
	char *data;
	struct timespec start, end;
	void *buf1 = NULL;
	char *buf;
	int size, count;
	pthread_t *pid;
	int *pdata;
	void *thread_ret;
	FILE *output;
	char fs_type[20];
	char file_size_num[20];
	char xip_enabled[20];
	char use_nvp[20];
	char filename[60];

	if (argc < 2)
		num_threads = 1;
	else
		num_threads = atoi(argv[1]);

	pid = (pthread_t *)malloc(sizeof(pthread_t) * num_threads); // pthread id array
	pdata = (int *)malloc(sizeof(int) * num_threads); // pthread data array

	posix_memalign(&buf1, SIZE, SIZE);

	buf = (char *)buf1;

	fd = open("/mnt/ramdisk/test1", O_CREAT | O_RDWR); 
	data = (char *)mmap(NULL, SIZE, PROT_WRITE, MAP_SHARED, fd, 0);
	origin_data = data;

	for (size = start_size; size <= SIZE / num_threads; size <<= 1) {
		buf = (char *)buf1;
		memset(buf, c, SIZE);
		c++;

		clock_gettime(CLOCK_MONOTONIC, &start);
		for (i = 0; i < num_threads; i++) {
			pdata[i] = (size << 8) + i;
//			printf("0x%x\n", pdata[i]);
			pthread_create(&pid[i], NULL, mmap_transfer, &pdata[i]);
		}

//		clock_gettime(CLOCK_MONOTONIC, &start);
		for (i = 0; i < num_threads; i++)
			pthread_join(pid[i], &thread_ret);

		msync(origin_data, SIZE, MS_ASYNC);
		clock_gettime(CLOCK_MONOTONIC, &end);

		time = (end.tv_sec - start.tv_sec) * 1e9 + (end.tv_nsec - start.tv_nsec);
		printf("Mmap: size %d bytes, %d threads,\t %ld nanoseconds,\t Bandwidth %f MB/s.\n", size, num_threads, time, 4.0 * 1e9 / time);
	}

	munmap(origin_data, SIZE);
	close(fd);
	free(buf1);
	free(pid);
	free(pdata);
	return 0;
}
