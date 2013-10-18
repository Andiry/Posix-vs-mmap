#include<stdio.h>
#include<stdlib.h>
#include<malloc.h>
#include<fcntl.h>
#include<time.h>
#include<sys/mman.h>
#include<string.h>

#define SIZE	(4 * 1024 * 1024) 

int main(void)
{
	int fd, i;
	unsigned long time;
	char c = 'a';
	char *data = NULL, *origin_data;
	struct timespec start, end;
	void *buf1 = NULL;
	char *buf;
	int start_size, count;

	posix_memalign(&buf1, SIZE, SIZE);

	buf = (char *)buf1;

	for (start_size = 1; start_size <= SIZE; start_size <<= 1) {
		buf = (char *)buf1;
		memset(buf, c, start_size);
		c++;

		fd = open("/mnt/ramdisk/test2", O_CREAT | O_RDWR); 
		data = (char *)mmap(NULL, SIZE, PROT_WRITE, MAP_SHARED, fd, 0);
		origin_data = data;

		count = SIZE / start_size;
		clock_gettime(CLOCK_MONOTONIC, &start);
		for (i = 0; i < count; i++) {
			memcpy(data, buf, start_size);
			data += start_size;
			buf += start_size;
		}

		msync(origin_data, SIZE, MS_ASYNC);
		clock_gettime(CLOCK_MONOTONIC, &end);
		time = (end.tv_sec - start.tv_sec) * 1e9 + (end.tv_nsec - start.tv_nsec);
		printf("Mmap: size %d bytes, %d times,\t %ld nanoseconds,\t Bandwidth %f MB/s.\n", start_size, count, time, 4.0 * 1e9 / time);
		munmap(origin_data, SIZE);
		close(fd);
	}

	free(buf1);
	return 0;
}
