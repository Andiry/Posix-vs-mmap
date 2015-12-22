#define _GNU_SOURCE

#include<stdio.h>
#include<fcntl.h>
#include<unistd.h>
#include<time.h>
#include<string.h>
#include<malloc.h>
#include<stdlib.h>
#include<sys/mman.h>
#include<sys/time.h>

#define END_SIZE	(4UL * 1024 * 1024) 

const int start_size = 512;

int main(int argc, char **argv)
{
	int fd, i, file_count;
	unsigned long long FILE_SIZE;
	size_t len;
	char c;
	char unit;
	int size;
	size_t ret;
	unsigned long long count;
	void *buf1 = NULL;
	char *buf, *buf2;
	char file_size_num[20];
	char filename[60];
	char *dir = "/mnt/ramdisk/file_";
	int req_size;
	int num_files;
	long long time;
	struct timespec start, end;

	if (argc < 4) {
		printf("Usage: ./write_files $NUM_FILES $FILE_SIZE $REQ_SIZE\n");
		return 0;
	}

	num_files = atoi(argv[1]);

	strcpy(file_size_num, argv[2]);
	len = strlen(file_size_num);
	unit = file_size_num[len - 1];
	file_size_num[len - 1] = '\0';
	FILE_SIZE = atoll(file_size_num);
	switch (unit) {
	case 'K':
	case 'k':
		FILE_SIZE *= 1024;
		break;
	case 'M':
	case 'm':
		FILE_SIZE *= 1048576;
		break;
	case 'G':
	case 'g':
		FILE_SIZE *= 1073741824;
		break;
	default:
		printf("ERROR: FILE_SIZE should be #K/M/G format.\n");
		return 0;
		break;
	}

	if (FILE_SIZE < 4096)
		FILE_SIZE = 4096;
	if (FILE_SIZE > 2147483648) // RAM disk size
		FILE_SIZE = 2147483648;

	req_size = atoi(argv[3]);
	if (req_size > END_SIZE)
		req_size = END_SIZE;

	c = 'a';

	if (posix_memalign(&buf1, END_SIZE, END_SIZE)) { // up to 64MB
		printf("ERROR - POSIX NOMEM!\n");
		return 0;
	}

	buf = (char *)buf1;
	buf2 = malloc(END_SIZE);
	memset(buf, c, req_size);
	size = req_size;
	count = FILE_SIZE / size;

	clock_gettime(CLOCK_MONOTONIC, &start);
	for (file_count = 0; file_count < num_files; file_count++) {
		sprintf(filename, "%s%d", dir, file_count);
		fd = open(filename, O_CREAT | O_RDWR, 0640);

		lseek(fd, 0, SEEK_SET);
		for (i = 0; i < count; i++) {
			ret = write(fd, buf, size);
			if (ret != size)
				printf("ERROR: size incorrect: required %d, "
					"returned %lu\n", size, ret);
		}

		close(fd);
	}

	clock_gettime(CLOCK_MONOTONIC, &end);
	time = (end.tv_sec - start.tv_sec) * 1e9 + (end.tv_nsec - start.tv_nsec);
	printf("write_files: %d files, %lld nanoseconds, per file latency %lld nanoseconds, "
		"per request latency %lld nanoseconds.\n",
		num_files, time, time / num_files, time / (num_files * count));
	free(buf1);
	free(buf2);
	return 0;
}
