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
	int i;
	ssize_t ret;

	buf = malloc(4096);
	memset(buf, 'd', 4096);
	buf1 = malloc(4096 * 6);
	memset(buf1, 'a', 4096 * 6);

	fd = open("/mnt/ramdisk/test1", O_RDWR | O_CREAT, 0640);
//	fd1 = open("/dev/bankshot2Ctrl0", O_RDWR);

	for (i = 0; i < 4; i++)
		read(fd, buf, 4096);

out:
	close(fd);
	free(buf);
	free(buf1);

	return 0;
}
