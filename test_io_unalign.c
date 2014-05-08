#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <malloc.h>

int main(void)
{
	int fd;
	char *buf, *buf1;
	ssize_t ret;

//	buf = malloc(1024);
//	memset(buf, 'd', 1024);
	buf = malloc(1024);
	memset(buf, 'd', 1024);
	buf1 = malloc(8192 + 4096);
	memset(buf1, 'x', 8192 + 4096);

	fd = open("/mnt/ramdisk/test1", O_RDWR | O_CREAT, 0640);

	ret = pwrite(fd, buf1, 8192 + 4096, 0);
	printf("Posix write to file ret: %lu\n", ret);
	ret = pwrite(fd, buf, 1024, 1024);
	printf("Posix write to file ret: %lu\n", ret);
	printf("buf1: %c %c %c %c\n", buf1[0], buf1[1023], buf1[1024], buf1[8192]);
	ret = pread(fd, buf1, 1024, 1024);
	printf("Posix read to file ret: %lu\n", ret);
	printf("buf1: %c %c %c %c\n", buf1[0], buf1[1023], buf1[1024], buf1[8192]);

	close(fd);
	free(buf);
	free(buf1);

	return 0;
}
