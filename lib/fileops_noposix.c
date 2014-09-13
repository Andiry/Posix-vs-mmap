// a module which repalces the standart POSIX functions with memory mapped equivalents

#include<stdio.h>
#include<unistd.h>
#include<fcntl.h>
#include<time.h>
#include<string.h>
#include<errno.h>
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

#define IS_ERR(x) ((unsigned long)(x) >= (unsigned long)-4095)

struct NVFile
{
	int fd;
	int valid;
	int canRead;
	int canWrite;
	volatile size_t offset;
	size_t length;
	size_t maplength;
	char *volatile data;
};

struct NVFile* fd_lookup;

#if 0

ssize_t read1(int fd, void **buf, size_t size)
{
	struct NVFile *nvf = &fd_lookup[fd];
	ssize_t length;

	if (nvf->offset >= nvf->length)
		return 0;

	*buf = nvf->data + nvf->offset;
	length = nvf->length >= nvf->offset + size ?
			size : nvf->length - nvf->offset;

	if (length != size)
		printf("read ERROR: request %lu, return %lu, offset %lu, "
			"file length %lu\n", size, length, nvf->offset,
			nvf->length);

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

off_t lseek1(int fd, off_t offset, int whence)
{
	struct NVFile *nvf = &fd_lookup[fd];

	switch (whence) {
	case SEEK_SET:
		nvf->offset = offset;
		break;
	case SEEK_CUR:
		nvf->offset += offset;
		break;
	case SEEK_END:
		nvf->offset = nvf->length + offset;
		break;
	default:
		break;
	}

	return nvf->offset;
}

int open1(const char* path, int oflag, mode_t mode)
{
	int fd;
	struct stat file_st;
	struct NVFile *nvf;
	int max_perms;

	fd = open(path, oflag, mode);
	stat(path, &file_st);
	nvf = &fd_lookup[fd];

	nvf->length = file_st.st_size;
	nvf->maplength = 0;
	nvf->valid = 1;
	nvf->offset = 0;

	if (oflag & O_RDWR)
		max_perms = PROT_READ | PROT_WRITE;
	else if (oflag & O_WRONLY)
		max_perms = PROT_WRITE;
	else if (oflag & O_RDONLY)
		max_perms = PROT_READ;
	else
		max_perms = 0;

	nvf->data = (char *)mmap(NULL, nvf->length, max_perms,
				MAP_SHARED | MAP_POPULATE,
				fd, 0);

	if (IS_ERR(nvf->data))
		printf("mmap error: %d\n", nvf->data);

	nvf->maplength = nvf->length;

	return fd;
}

int close1(int fd)
{
	struct NVFile *nvf;

	nvf = &fd_lookup[fd];
	nvf->valid = 0;
	munmap(nvf->data, nvf->maplength);

	close(fd);
	return 0;
}

#else

ssize_t read1(int fd, char **buf, size_t size)
{
	struct NVFile *nvf = &fd_lookup[fd];
	ssize_t length;
	int max_perms;

	if (nvf->offset >= nvf->length)
		return 0;

	if (nvf->canRead && nvf->canWrite)
		max_perms = PROT_READ | PROT_WRITE;
	else if (nvf->canWrite)
		max_perms = PROT_WRITE;
	else if (nvf->canRead)
		max_perms = PROT_READ;
	else
		max_perms = 0;

	length = nvf->length >= nvf->offset + size ?
			size : nvf->length - nvf->offset;

	*buf = (char *)mmap(NULL, length, max_perms,
				MAP_SHARED | MAP_POPULATE,
				fd, nvf->offset);

	if (IS_ERR(*buf))
		printf("read1 mmap error: %s, length %lu, offset %lu\n",
			strerror(errno), length, nvf->offset % 4096);

	if (length != size)
		printf("read ERROR: request %lu, return %lu, offset %lu, "
			"file length %lu\n", size, length, nvf->offset,
			nvf->length);

	nvf->offset += length;

	return length;
}

ssize_t write1(int fd, char **buf, size_t size)
{
	return size;
}

ssize_t pread1(int fd, char **buf, size_t size, off_t offset)
{
	struct NVFile *nvf = &fd_lookup[fd];
	ssize_t length;
	int max_perms;

	if (offset >= nvf->length)
		return 0;

	if (nvf->canRead && nvf->canWrite)
		max_perms = PROT_READ | PROT_WRITE;
	else if (nvf->canWrite)
		max_perms = PROT_WRITE;
	else if (nvf->canRead)
		max_perms = PROT_READ;
	else
		max_perms = 0;

	length = nvf->length >= offset + size ?
			size : nvf->length - offset;

	*buf = (char *)mmap(NULL, length, max_perms,
				MAP_SHARED | MAP_POPULATE,
				fd, offset);

	if (IS_ERR(*buf))
		printf("pread1 mmap error: %s, length %lu, offset %lu\n",
			strerror(errno), length, offset % 4096);

	if (length != size)
		printf("read ERROR: request %lu, return %lu, offset %lu, "
			"file length %lu\n", size, length, nvf->offset,
			nvf->length);

	return length;

}

ssize_t pwrite1(int fd, char **buf, size_t size, off_t offset)
{
	return size;
}

off_t lseek1(int fd, off_t offset, int whence)
{
	struct NVFile *nvf = &fd_lookup[fd];

	switch (whence) {
	case SEEK_SET:
		nvf->offset = offset;
		break;
	case SEEK_CUR:
		nvf->offset += offset;
		break;
	case SEEK_END:
		nvf->offset = nvf->length + offset;
		break;
	default:
		break;
	}

	return nvf->offset;
}

int open1(const char* path, int oflag, mode_t mode)
{
	int fd;
	struct stat file_st;
	struct NVFile *nvf;
	int max_perms;

	fd = open(path, oflag, mode);
	stat(path, &file_st);
	nvf = &fd_lookup[fd];

	nvf->length = file_st.st_size;
	nvf->maplength = 0;
	nvf->valid = 1;
	nvf->offset = 0;

	nvf->canRead = nvf->canWrite = 0;

	if (oflag & O_RDWR)
		nvf->canRead = nvf->canWrite = 1;
	else if (oflag & O_WRONLY)
		nvf->canWrite = 1;
	else if (oflag & O_RDONLY)
		nvf->canRead = 1;

	return fd;
}

int close1(int fd)
{
	struct NVFile *nvf;

	nvf = &fd_lookup[fd];
	nvf->valid = 0;

	close(fd);
	return 0;
}

#endif

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
