#define _GNU_SOURCE

#include<stdio.h>
#include<fcntl.h>
#include<time.h>
#include<string.h>
#include<malloc.h>
#include<stdlib.h>
#include<sys/mman.h>
#include<sys/time.h>

#define END_SIZE	(4UL * 1024 * 4096) 

const int start_size = 4096;

int main(int argc, char **argv)
{
	int fd, i, j;
	long long time, time1;
	unsigned long long FILE_SIZE;
	size_t len;
	off_t offset;
	char c = 'a';
	char unit;
	struct timespec start, end;
	struct timeval begin, finish;
	struct timezone tz;
	int size;
	unsigned long long count;
	void *buf1 = NULL;
	char *buf;
	FILE *output;
	char fs_type[20];
	char quill_enabled[40];
	char file_size_num[20];
	char filename[60];
	int enable_ftrace;
	long long *recnum= 0;
	unsigned long long big_rand;
	off64_t offset64 = 0;

	if (argc < 6) {
		printf("Usage: ./random_perf_test_to_ram $FS $SCNEARIO $ENABLE_FTRACE $FILE_SIZE $filename\n");
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
	output = fopen(filename, "a");

	if (posix_memalign(&buf1, END_SIZE, END_SIZE)) { // up to 64MB
		printf("ERROR - POSIX NOMEM!\n");
		return 0;
	}

	buf = (char *)buf1;
//	fd = open("/mnt/ramdisk/test1", O_CREAT | O_RDONLY, 0640); 
//	fd = open("/dev/null", O_WRONLY, 0640); 
//	fd = open("/dev/zero", O_RDONLY, 0640); 
	printf("fd: %d\n", fd);
//	start_size = atoi(argv[2]);
	enable_ftrace = atoi(argv[3]);

        srand48(0);

	size = start_size;
	count = FILE_SIZE / size;
        recnum = (long long *)malloc(sizeof(*recnum) * count);
        /* pre-compute random sequence based on 
		Fischer-Yates (Knuth) card shuffle */
	for(i = 0; i < count; i++){
		recnum[i] = i;
	}
	for(i = 0; i < count; i++) {
		long long tmp;
		big_rand = lrand48();
		big_rand = big_rand % count;
		tmp = recnum[i];
		recnum[i] = recnum[big_rand];
		recnum[big_rand] = tmp;
	}

	for (j = 0; j < 3; j++) {
		fd = open("/mnt/ramdisk/test1", O_CREAT | O_RDWR, 0640); 
		size = start_size;
//		size = atoi(argv[2]);
		memset(buf, '\0', size);
		if (read(fd, buf, size) != size)
			printf("Error reading\n");
		lseek(fd, 0, SEEK_SET);
		offset = 0;

		gettimeofday(&begin, &tz);
		clock_gettime(CLOCK_MONOTONIC, &start);
	
		for (i = 0; i < count; i++) {
			offset64 = size * (long long)recnum[i];

			if(lseek(fd, offset64, SEEK_SET )<0)
			{
				perror("lseek");
				exit(68);
			}

			if (j < 2) {
				if (read(fd, buf, size) != size)
					printf("ERROR read!\n");
			} else {
				if (write(fd, buf, size) != size)
					printf("ERROR write!\n");
			}
//			pread(fd, buf, size, offset);
//			if (pread(fd, buf, size, offset) != size)
//				printf("ERROR!\n");
//			offset += size;
		}
		clock_gettime(CLOCK_MONOTONIC, &end);
		gettimeofday(&finish, &tz);

		time = (end.tv_sec - start.tv_sec) * 1e9 + (end.tv_nsec - start.tv_nsec);
		time1 = (finish.tv_sec - begin.tv_sec) * 1e6 + (finish.tv_usec - begin.tv_usec);
		printf("Read: Size %d bytes,\t %lld times,\t %lld nanoseconds, \t %lld us, \t latency %lld nanoseconds, \t Bandwidth %f MB/s.\n", size, count, time, time1, time / count, FILE_SIZE * 1024.0 / time);
		printf("Read process %lld microseconds\n", time1);
		fprintf(output, "%s,%s,%d,%lld,%lld,%lld,%f,%lld\n", fs_type, quill_enabled, size, FILE_SIZE, count, time, FILE_SIZE * 1.0 / time, time / count);
		close(fd);
	}

	fclose(output);
//	close(fd);
	free(buf1);
	free(recnum);
	return 0;
}
