#define _GNU_SOURCE

#include<stdio.h>
#include<unistd.h>
#include<fcntl.h>
#include<time.h>
#include<string.h>
#include<malloc.h>
#include<stdlib.h>
#include<stdint.h>
#include<stdbool.h>
#include<pthread.h>
#include<sys/mman.h>
#include<sys/time.h>

int main(void)
{
	int fd;
	int ret;
	int c = 32;
	size_t size;

	fd = open("/mnt/ramdisk/test2", O_CREAT | O_RDWR, 0640);

	size = write(fd, &c, 4);
	printf("%d %u\n", fd, size);
	ret = fallocate(fd, 0, 0, 10485760);

	printf("%d\n", ret);
	close(fd);
	return ret;
}
