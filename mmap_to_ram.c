#include<stdio.h>
#include<stdlib.h>
#include<malloc.h>
#include<unistd.h>
#include<fcntl.h>
#include<time.h>
#include<sys/mman.h>
#include<string.h>
#include<pthread.h>

#define END_SIZE	(64 * 1024 * 1024) 
//const unsigned long long FILE_SIZE = 1L * 1024 * 1024 * 1024; 

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
	unsigned long long time;
	unsigned long long FILE_SIZE;
	char c = 'A';
	char *origin_data;
	struct timespec start, end;
	void *buf1 = NULL;
	int size, count;
	pthread_t *pid;
	int *pdata;
	void *thread_ret;
	FILE *output;
	char fs_type[20];
	char xip_enabled[20];
	char use_nvp[20];
	char filename[60];
	struct tm *local;

	if (argc < 6) {
		printf("Usage: ./mmap_to_ram $FS $XIP $Quill $FILE_SIZE $filename\n");
		return 0;
	}

	strcpy(fs_type, argv[1]);

	if (!strcmp(argv[2], "0"))
		strcpy(xip_enabled, "No_XIP");
	else
		strcpy(xip_enabled, "XIP");

	if (!strcmp(argv[3], "0"))
		strcpy(use_nvp, "Posix-mmap");
	else
		strcpy(use_nvp, "Quill-mmap");

	FILE_SIZE = atoll(argv[4]);

	strcpy(filename, argv[5]);
	output = fopen(filename, "a");

	posix_memalign(&buf1, END_SIZE, END_SIZE);

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
		printf("Mmap: size %d bytes, %d times,\t %lld nanoseconds,\t Bandwidth %f MB/s.\n", size, count, time, FILE_SIZE * 1024.0 / time);
		fprintf(output, "%s,%s,%s,%d,%lld,%d,%lld,%f\n", fs_type, use_nvp, xip_enabled, size, FILE_SIZE, count, time, FILE_SIZE * 1.0 / time);
	}

	fclose(output);
	munmap(origin_data, FILE_SIZE);
	close(fd);
	free(buf1);
	return 0;
}
