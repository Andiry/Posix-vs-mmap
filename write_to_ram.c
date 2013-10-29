#define _GNU_SOURCE

#include<stdio.h>
#include<fcntl.h>
#include<time.h>
#include<string.h>
#include<malloc.h>
#include<stdlib.h>
#include<sys/mman.h>

#define END_SIZE	(1 * 1024 * 1024) 
#define FILE_SIZE	(46 * 1024 * 1024) 

const int start_size = 512;

int main(int argc, char **argv)
{
	int fd, i;
	unsigned long time1;
	char c = 'a';
	struct timespec start, end;
	int size, count;
	void *buf1 = NULL;
	char *buf;
	FILE *output;
	char xip_enabled[20];
	char quill_enabled[20];
	struct tm *local;

	if (argc < 3)
		return 0;

	if (!strcmp(argv[1], "0"))
		strcpy(xip_enabled, "No_XIP");
	else
		strcpy(xip_enabled, "XIP");

	if (!strcmp(argv[2], "0"))
		strcpy(quill_enabled, "Posix");
	else
		strcpy(quill_enabled, "Quill");

	output = fopen("results.csv", "a");

	posix_memalign(&buf1, END_SIZE, END_SIZE); // up to 1MB
	if (!buf1) {
		printf("ERROR!\n");
		return 0;
	}

	buf = (char *)buf1;
	fd = open("/mnt/ramdisk/test1", O_CREAT | O_RDWR | O_DIRECT); 

	for (size = start_size; size <= END_SIZE; size <<= 1) {
		memset(buf, c, size);
		c++;
		lseek(fd, 0, SEEK_SET);

		count = FILE_SIZE / size;
		clock_gettime(CLOCK_MONOTONIC, &start);
		for (i = 0; i < count; i++)
			write(fd, buf, size);

		clock_gettime(CLOCK_MONOTONIC, &end);
		time1 = (end.tv_sec - start.tv_sec) * 1e9 + (end.tv_nsec - start.tv_nsec);
		printf("Size %d bytes,\t %d times,\t %ld nanoseconds,\t Bandwidth %f MB/s.\n", size, count, time1, FILE_SIZE * 1024.0 / time1);
		fprintf(output, "%s,%s,%d,%d,%ld,%f\n", quill_enabled, xip_enabled, size, count, time1, FILE_SIZE * 1024.0 / time1);
	}

	fclose(output);
	close(fd);
	free(buf1);
	return 0;
}
