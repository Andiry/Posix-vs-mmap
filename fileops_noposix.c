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

ssize_t read1(int fd, void *buf, size_t size)
{
	return size;
}

ssize_t write1(int fd, void *buf, size_t size)
{
	return size;
}

ssize_t pread1(int fd, void *buf, size_t size, off_t offset)
{
	return size;
}

ssize_t pwrite1(int fd, void *buf, size_t size, off_t offset)
{
	return size;
}

__attribute__((constructor)) void init(void)
{
	printf("Start.\n");
}

__attribute__((destructor)) void fini(void)
{
	printf("Finish.\n");
}
