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
	int fd, i;
	char c;
	int size;
	void *buf1 = NULL;
	char *buf;
	int req_size;
	int offset;
	struct timespec begin, finish;
	int retry;
	long long time1;

	if (argc < 4) {
		printf("Usage: ./rewrite $REQ_SIZE $OFFSET $REWRITE_TIME\n");
		return 0;
	}

	retry = atoi(argv[3]);
	c = 'a' + (retry % 26);

	if (posix_memalign(&buf1, END_SIZE, END_SIZE)) { // up to 64MB
		printf("ERROR - POSIX NOMEM!\n");
		return 0;
	}

	buf = (char *)buf1;
	fd = open("/mnt/ramdisk/test1", O_CREAT | O_RDWR, 0640); 
	printf("fd: %d\n", fd);
	req_size = atoi(argv[1]);
	if (req_size > END_SIZE)
		req_size = END_SIZE;

	offset = atoi(argv[2]);
	size = req_size;
	memset(buf, c, size);

	clock_gettime(CLOCK_MONOTONIC, &begin);
	for (i = 0; i < retry; i++) {
		lseek(fd, offset, SEEK_SET);

		if (write(fd, buf, size) != size)
			printf("ERROR\n");
	}
	clock_gettime(CLOCK_MONOTONIC, &finish);

	time1 = (finish.tv_sec * 1e9 + finish.tv_nsec) - (begin.tv_sec * 1e9 + begin.tv_nsec);
	printf("write %lld ns, average %lld ns\n", time1, time1 / retry);

//	fsync(fd);
	close(fd);

	free(buf1);
	return 0;
}
