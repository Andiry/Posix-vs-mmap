// a module which repalces the standart POSIX functions with memory mapped equivalents

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

#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>


//#include "my_memcpy_nocache.h"

// TODO: manual prefaulting sometimes segfaults
#define MANUAL_PREFAULT 0
#define MMAP_PREFAULT 1

#define MAX_MMAP_SIZE	2097152
#define	ALIGN_MMAP_DOWN(addr)	((addr) & ~(MAX_MMAP_SIZE - 1))

#define DO_ALIGNMENT_CHECKS 0

#define OPEN_MAX	1024

struct NVFile
{
	int fd;
	volatile size_t offset;
	size_t length;
	size_t maplength;
	char *volatile data;
};

struct NVFile* fd_lookup;

ssize_t read1(int fd, void **buf, size_t size)
{
	struct NVFile *nvf = &fd_lookup[fd];
	ssize_t length;

	if (nvf->offset >= nvf->length)
		return 0;

	*buf = nvf->data + nvf->offset;
	length = nvf->length >= nvf->offset + size ?
			size : nvf->length - nvf->offset;

	nvf->offset += length;

	return length;
}

ssize_t write1(int fd, void **buf, size_t size)
{
	return size;
}

ssize_t pread1(int fd, void **buf, size_t size, off_t offset)
{
	struct NVFile *nvf = &fd_lookup[fd];
	ssize_t length;

	if (offset >= nvf->length)
		return 0;

	*buf = nvf->data + offset;
	length = nvf->length >= offset + size ?
			size : nvf->length - offset;

	return length;

}

ssize_t pwrite1(int fd, void **buf, size_t size, off_t offset)
{
	return size;
}

__attribute__((constructor)) void init(void)
{
	printf("Start.\n");
	fd_lookup = (struct NVFile *)calloc(OPEN_MAX, sizeof(struct NVFile));
}

__attribute__((destructor)) void fini(void)
{
	printf("Finish.\n");
	free(fd_lookup);
}
