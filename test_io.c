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
	char *buf;
	ssize_t ret;

	buf = malloc(4096 * 6);
	memset(buf, 'c', 4096 * 6);

	fd = open("/mnt/ramdisk/test1", O_RDWR | O_CREAT, 0640);

	printf("Posix write to file: 0 - 5\n");
	ret = pwrite(fd, buf, 4096 * 6, 0);
	printf("Posix write to file: 0 - 5 ret: %d\n", ret);

	printf("Posix read to file: 2 - 4\n");
	ret = pread(fd, buf, 4096 * 3, 4096 * 2);
	printf("Posix read to file: 2 - 4 ret: %d\n", ret);

	printf("Posix write to file: 1 - 3\n");
	ret = pwrite(fd, buf, 4096 * 3, 4096);
	printf("Posix write to file: 1 - 3 ret: %d\n", ret);

	printf("Posix write to file: 4 - 8\n");
	ret = pwrite(fd, buf, 4096 * 5, 4096 * 4);
	printf("Posix write to file: 4 - 8 ret: %d\n", ret);

	printf("Posix read to file: 2 - 5\n");
	ret = pread(fd, buf, 4096 * 4, 4096 * 2);
	printf("Posix read to file: 2 - 5 ret: %d\n", ret);

	close(fd);
	free(buf);

	return 0;
}
