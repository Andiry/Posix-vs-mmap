// a module which repalces the standart POSIX functions with memory mapped equivalents

#include<stdio.h>
#include<unistd.h>
#include<fcntl.h>
#include<time.h>
#include<assert.h>
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
	unsigned long *root;
	unsigned int height;
};

struct NVFile* fd_lookup;
int hit_count;
int mmap_count;

#define	MAX_MMAP_SIZE	2097152
#define	ALIGN_MMAP_DOWN(addr)	((addr) & ~(MAX_MMAP_SIZE - 1))

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

static void free_btree(unsigned long *root, unsigned long height)
{
	int i;

	if (height == 0) return;

	for (i = 0; i < 1024; i++) {
		if (root[i]) {
			free_btree((unsigned long *)root[i],
					height - 1);
			root[i] = 0;
		}
	}

	free(root);
}

static void free_file(struct NVFile *nvf)
{
	unsigned int height = nvf->height;
	unsigned long *root = nvf->root;

	free_btree(root, height);

	nvf->height = 0;
	if (nvf->root && height == 0)
		free(nvf->root);

	nvf->root = NULL;
}

static void init_file(struct NVFile *nvf)
{
	int i;

	if (!nvf->root)
		nvf->root = malloc(1024 * sizeof(unsigned long));

	for (i = 0 ; i < 1024; i++)
		nvf->root[i] = 0;
}

static unsigned long calculate_capacity(unsigned int height)
{
	unsigned long capacity = MAX_MMAP_SIZE;

	while (height) {
		capacity *= 1024;
		height--;
	}

	return capacity;
}

static unsigned int calculate_new_height(off_t offset)
{
	unsigned int height = 0;
	off_t temp_offset = offset / ((unsigned long)1024 * MAX_MMAP_SIZE);

	while (temp_offset) {
		temp_offset /= 1024;
		height++;
	}

	return height;
}

static ssize_t get_mmap_address(int fd, off_t offset, size_t length,
		char **buf, int max_perms)
{
	struct NVFile *nvf = &fd_lookup[fd];
	int i, index;
	unsigned int height = nvf->height;
	unsigned int new_height;
	unsigned long capacity = MAX_MMAP_SIZE;
	unsigned long *root = nvf->root;
	unsigned long start_addr;
	off_t start_offset = offset;
	size_t result;

	do {
		capacity = calculate_capacity(height);
		index = start_offset / capacity;
		if (index >= 1024 || root[index] == 0)
			goto not_found;

		if (height)
			root = (unsigned long *)root[index];
		else
			start_addr = root[index];

		start_offset = start_offset % capacity;
	} while(height--);

	hit_count++;
	goto done;

not_found:
	start_offset = ALIGN_MMAP_DOWN(offset);

	start_addr = (unsigned long)mmap(NULL, MAX_MMAP_SIZE, max_perms,
				MAP_SHARED | MAP_POPULATE,
				fd, start_offset);
	mmap_count++;

	if (start_addr == MAP_FAILED) {
		printf("mmap failed for fd %d\n", fd);
		assert(0);
	}

	height = nvf->height;
	new_height = calculate_new_height(offset);

	if (height < new_height) {
		while (height < new_height) {
			unsigned long old_root = (unsigned long)nvf->root;
			nvf->root = malloc(1024 * sizeof(unsigned long));
			for (i = 0;  i < 1024; i++)
				nvf->root[i] = 0;
			nvf->root[0] = (unsigned long)old_root;
			height++;
		}

		nvf->height = new_height;
		height = new_height;
	}

	root = nvf->root;
	do {
		capacity = calculate_capacity(height);
		index = start_offset / capacity;
		if (height) {
			if (root[index] == 0) {
				root[index] = (unsigned long)malloc(1024 *
						sizeof(unsigned long));
				root = (unsigned long *)root[index];
				for (i = 0; i < 1024; i++)
					root[i] = 0;
			} else {
				root = (unsigned long *)root[index];
			}
		} else {
			root[index] = start_addr;
		}
		start_offset = start_offset % capacity;
	} while(height--);

done:
	*buf = (char *)(start_addr + offset % MAX_MMAP_SIZE);
	result = MAX_MMAP_SIZE - (offset % MAX_MMAP_SIZE);
	if (result > length)
		result = length;

	return result;
}

static ssize_t do_pread(int fd, char **buf, size_t size, off_t offset)
{
	struct NVFile *nvf = &fd_lookup[fd];
	ssize_t length, result;
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

	if (length <= 0)
		return 0;

	result = get_mmap_address(fd, offset, length, buf, max_perms);

	if (IS_ERR(*buf))
		printf("do_pread mmap error: %s, length %lu, offset %lu\n",
			strerror(errno), length, offset % 4096);

	if (result != size)
		printf("do_pread ERROR: request %lu, return %lu, offset %lu, "
			"file length %lu\n", size, result, nvf->offset,
			nvf->length);

	return result;

}

ssize_t read1(int fd, char **buf, size_t size)
{
	struct NVFile *nvf = &fd_lookup[fd];
	ssize_t length;

	length = do_pread(fd, buf, size, nvf->offset);

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

	length = do_pread(fd, buf, size, offset);

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

	init_file(nvf);

	return fd;
}

int close1(int fd)
{
	struct NVFile *nvf;

	nvf = &fd_lookup[fd];
	nvf->valid = 0;

	free_file(nvf);

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
	printf("Finish. hit %d, mmap %d\n", hit_count, mmap_count);
	free(fd_lookup);
}
