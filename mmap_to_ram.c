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

int main(int argc, char ** argv)
{
	int fd, i;
	unsigned long long time;
	unsigned long long FILE_SIZE;
	size_t len;
	char c = 'A';
	char unit;
	char *origin_data;
	char *data;
	struct timespec start, end;
	void *buf1 = NULL;
	char *buf;
	int size, count;
	FILE *output;
	char fs_type[20];
	char file_size_num[20];
	char xip_enabled[20];
	char use_nvp[20];
	char filename[60];

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

	if (FILE_SIZE < END_SIZE)
		FILE_SIZE = END_SIZE;
	if (FILE_SIZE > 2147483648) // RAM disk size
		FILE_SIZE = 2147483648;

	strcpy(filename, argv[5]);
	output = fopen(filename, "a");

	posix_memalign(&buf1, END_SIZE, END_SIZE);

	buf = (char *)buf1;

	fd = open("/mnt/ramdisk/test1", O_CREAT | O_RDWR); 
	data = (char *)mmap(NULL, FILE_SIZE, PROT_WRITE, MAP_SHARED, fd, 0);
	origin_data = data;

	for (size = start_size; size <= END_SIZE; size <<= 1) {
		memset(buf, c, size);
		c++;
		data = origin_data;
		count = FILE_SIZE / size;

		clock_gettime(CLOCK_MONOTONIC, &start);
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
