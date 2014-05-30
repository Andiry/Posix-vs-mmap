#define _GNU_SOURCE

#include<stdio.h>
#include<fcntl.h>
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
	int fd, i, j;
	long long time, time1;
	unsigned long long FILE_SIZE;
	size_t len;
	off_t offset;
	char c, origin_c;
	char unit;
	struct timespec start, end;
	struct timeval begin, finish;
	struct timezone tz;
	int size;
	unsigned long long count;
	void *buf1 = NULL;
	char *buf, *buf2;
	FILE *output;
	char fs_type[20];
	char quill_enabled[40];
	char file_size_num[20];
	char filename[60];
	int enable_ftrace;

	if (argc < 6) {
		printf("Usage: ./integrity_test1 $FS $SCNEARIO $ENABLE_FTRACE $FILE_SIZE $filename\n");
		return 0;
	}

	strcpy(fs_type, argv[1]);
	strcpy(quill_enabled, argv[2]);

	strcpy(file_size_num, argv[4]);
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

	strcpy(filename, argv[5]);
	c = filename[0];
	output = fopen(filename, "a");

	if (posix_memalign(&buf1, END_SIZE, END_SIZE)) { // up to 64MB
		printf("ERROR - POSIX NOMEM!\n");
		return 0;
	}

	buf = (char *)buf1;
	buf2 = malloc(END_SIZE);
	fd = open("/mnt/ramdisk/test1", O_CREAT | O_RDWR, 0640); 
//	fd = open("/dev/null", O_WRONLY, 0640); 
//	fd = open("/dev/zero", O_RDONLY, 0640); 
	printf("fd: %d\n", fd);
//	start_size = atoi(argv[2]);
	enable_ftrace = atoi(argv[3]);
	for (j = 0; j < 3; j++) {
	for (size = start_size; size <= END_SIZE; size <<= 1) {
//		size = 8192;
//		size = atoi(argv[2]);
		fd = open("/mnt/ramdisk/test1", O_CREAT | O_RDWR, 0640); 
		memset(buf, c, size);
		origin_c = c;
		lseek(fd, 0, SEEK_SET);
		offset = 0;
		count = FILE_SIZE / size;
		printf("Start c: %c\n", c);
		for (i = 0; i < count; i++) {
			write(fd, buf, size);
			c++;
			memset(buf, c, size);
		}

		close(fd);
		lseek(fd, 0, SEEK_SET);
		offset = 0;
//		if (enable_ftrace)
//			system("echo 1 > /sys/kernel/debug/tracing/tracing_on");
	
		fd = open("/mnt/ramdisk/test1", O_CREAT | O_RDWR, 0640); 
		gettimeofday(&begin, &tz);
		clock_gettime(CLOCK_MONOTONIC, &start);
		for (i = 0; i < count; i++) {
			read(fd, buf2, size);
			if ((buf2[0]) != origin_c)
				printf("ERROR!\n");
			origin_c++;
		}
		clock_gettime(CLOCK_MONOTONIC, &end);
		gettimeofday(&finish, &tz);
		close(fd);
		c++;

//		if (enable_ftrace)
//			system("echo 0 > /sys/kernel/debug/tracing/tracing_on");
		time = (end.tv_sec - start.tv_sec) * 1e9 + (end.tv_nsec - start.tv_nsec);
		time1 = (finish.tv_sec - begin.tv_sec) * 1e6 + (finish.tv_usec - begin.tv_usec);
		printf("Read: Size %d bytes,\t %lld times,\t %lld nanoseconds,\t latency %lld nanoseconds, \t Bandwidth %f MB/s.\n", size, count, time, time / count, FILE_SIZE * 1024.0 / time);
		printf("Read process %lld microseconds\n", time1);
		fprintf(output, "%s,%s,%d,%lld,%lld,%lld,%f,%lld\n", fs_type, quill_enabled, size, FILE_SIZE, count, time, FILE_SIZE * 1.0 / time, time / count);
	}
	}

	fclose(output);
	free(buf1);
	free(buf2);
	return 0;
}
