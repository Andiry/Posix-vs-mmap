#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <malloc.h>

#include "../bankshot2/kernel/bankshot2_cache.h"

int main(void)
{
	int fd, fd1;
	char *buf, *buf1;
	int num_free = 1;
	ssize_t ret;

	buf = malloc(4096 * 6);
	memset(buf, 'd', 4096 * 6);
	buf1 = malloc(4096 * 6);
	memset(buf1, 'a', 4096 * 6);

	fd = open("/mnt/ramdisk/test1", O_RDWR | O_CREAT, 0640);
	fd1 = open("/dev/bankshot2Ctrl0", O_RDWR);

	printf("Posix write to file: 0 - 5\n");
	ret = pwrite(fd, buf, 4096 * 6, 0);
	printf("Posix write to file: 0 - 5 ret: %d\n", ret);

	printf("Posix read from file: 0 - 1\n");
	ret = pread(fd, buf1, 4096, 0);
	printf("Posix read from file: 0 - 1 ret: %d, buf: %c\n", ret, buf1[0]);

//	memset(buf, 'f', 4096 * 6);
//	printf("Posix write to file: 1 - 3\n");
//	ret = pwrite(fd, buf, 4096 * 3, 4096);
//	printf("Posix write to file: 1 - 3 ret: %d\n", ret);

//	memset(buf, 'e', 4096 * 6);
//	printf("Posix write to file: 4 - 8\n");
//	ret = pwrite(fd, buf, 4096 * 5, 4096 * 4);
//	printf("Posix write to file: 4 - 8 ret: %d\n", ret);

//	printf("Posix read from file: 2 - 5\n");
//	ret = pread(fd, buf1, 4096 * 4, 4096 * 2);
//	printf("Posix read from file: 2 - 5 ret: %d, buf: %c %c %c %c\n",
//		ret, buf1[0], buf1[4096], buf1[8192], buf1[4096 * 3]);

//	ioctl(fd1, BANKSHOT2_IOCTL_FREE_BLOCKS, &num_free);

	printf("Posix read from file: 0 - 1\n");
	ret = pread(fd, buf1, 4096, 0);
	printf("Posix read from file: 0 - 1 ret: %d, buf: %c\n", ret, buf1[0]);

//	ioctl(fd1, BANKSHOT2_IOCTL_FREE_BLOCKS, &num_free);

	printf("Posix read from file: 0 - 1\n");
	ret = pread(fd, buf1, 4096, 0);
	printf("Posix read from file: 0 - 1 ret: %d, buf: %c\n", ret, buf1[0]);

//	printf("Posix read from file: 1 - 2\n");
//	ret = pread(fd, buf1, 4096 * 2, 4096 * 1);
//	printf("Posix read from file: 1 - 2, ret: %d, buf: %c\n", ret, buf1[0]);
//	printf("Posix read from file: 1 - 2\n");
//	ret = pread(fd, buf1, 4096 * 2, 4096 * 1);
//	printf("Posix read from file: 1 - 2, ret: %d, buf: %c\n", ret, buf1[0]);

out:
	close(fd);
	close(fd1);
	free(buf);
	free(buf1);

	return 0;
}
