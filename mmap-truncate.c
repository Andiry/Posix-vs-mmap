#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define MiB(a) ((a)*1024*1024)
#define KiB(a) ((a)*1024)

void err_exit(char *op)
{
	fprintf(stderr, "%s: %s\n", op, strerror(errno));
	exit(1);
}

int main(int argc, char *argv[])
{
	char *buffer = "HELLO WORLD!";
	char *data;
	int fd, err, ret = 0;

	if (argc < 2) {
		printf("Usage: %s <pmem file>\n", basename(argv[0]));
		exit(0);
	}

	fd = open(argv[1], O_RDWR|O_CREAT, S_IRUSR|S_IWUSR);
	if (fd < 0)
		err_exit("fd");

	pwrite(fd, buffer, strlen(buffer), 0);

	data = mmap(NULL, KiB(4), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	if (strncmp(buffer, data, strlen(buffer))) {
		fprintf(stderr, "strncmp mismatch: '%s' vs '%s'\n", buffer,
				data);
		ret = 1;
	}

	*(data) = 'S';

	ftruncate(fd, 0);

	*(data) = 'h';

	err = munmap(data, KiB(4));
	if (err < 0)
		err_exit("munmap");

	err = close(fd);
	if (err < 0)
		err_exit("close");

	return ret;
}
