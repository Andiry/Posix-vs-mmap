#define _GNU_SOURCE

#include<stdio.h>
#include<fcntl.h>
#include<unistd.h>
#include<time.h>
#include<string.h>
#include<malloc.h>
#include<stdlib.h>
#include<stdint.h>
#include<sys/mman.h>
#include<sys/time.h>

#define END_SIZE	(4UL * 1024 * 1024) 
#define FLUSH_ALIGN 64

typedef uint64_t uintptr_t;

const int start_size = 512;

#define _mm_clflush(addr)\
    asm volatile("clflush %0" : "+m" (*(volatile char *)addr));
#define _mm_clflushopt(addr)\
    asm volatile(".byte 0x66; clflush %0" : "+m" (*(volatile char *)addr));
#define _mm_clwb(addr)\
    asm volatile(".byte 0x66; xsaveopt %0" : "+m" (*(volatile char *)addr));
#define _mm_pcommit()\
    asm volatile(".byte 0x66, 0x0f, 0xae, 0xf8");

static void
flush_clflush(void *addr, size_t len)
{

    uintptr_t uptr;

    /*
     * Loop through cache-line-size (typically 64B) aligned chunks
     * covering the given range.
     */
    for (uptr = (uintptr_t)addr & ~(FLUSH_ALIGN - 1);
        uptr < (uintptr_t)addr + len; uptr += FLUSH_ALIGN)
        _mm_clflush((char *)uptr);
}

static void
flush_clflushopt(void *addr, size_t len)
{

    uintptr_t uptr;

    /*
     * Loop through cache-line-size (typically 64B) aligned chunks
     * covering the given range.
     */
    for (uptr = (uintptr_t)addr & ~(FLUSH_ALIGN - 1);
        uptr < (uintptr_t)addr + len; uptr += FLUSH_ALIGN) {
        _mm_clflushopt((char *)uptr);
    }
}

static void
flush_clwb(void *addr, size_t len)
{

    uintptr_t uptr;

    /*
     * Loop through cache-line-size (typically 64B) aligned chunks
     * covering the given range.
     */
    for (uptr = (uintptr_t)addr & ~(FLUSH_ALIGN - 1);
        uptr < (uintptr_t)addr + len; uptr += FLUSH_ALIGN) {
        _mm_clwb((char *)uptr);
    }
}

static void
drain_pcommit(void)
{

//    Func_predrain_fence();
    _mm_pcommit();
//    _mm_sfence();
}

int main(int argc, char **argv)
{
	int fd, i;
	unsigned long long FILE_SIZE;
	size_t len;
	char c;
	char unit;
	int size;
	unsigned long long count;
	void *buf1 = NULL;
	char *buf, *data, *addr;
	char file_size_num[20];
	char filename[60];
	int req_size;
	struct timespec begin, finish;
	long long time1;

	if (argc < 4) {
		printf("Usage: ./mmap_write $REQ_SIZE $FILE_SIZE $filename\n");
		return 0;
	}

	strcpy(file_size_num, argv[2]);
	len = strlen(file_size_num);
	unit = file_size_num[len - 1];
	file_size_num[len - 1] = '\0';
	FILE_SIZE = atoll(file_size_num);
	switch (unit) {
	case 'K':
	case 'k':
		FILE_SIZE *= 1024;
		break;
	case 'M':
	case 'm':
		FILE_SIZE *= 1048576;
		break;
	case 'G':
	case 'g':
		FILE_SIZE *= 1073741824;
		break;
	default:
		printf("ERROR: FILE_SIZE should be #K/M/G format.\n");
		return 0;
		break;
	}

	if (FILE_SIZE < 4096)
		FILE_SIZE = 4096;
	if (FILE_SIZE > 2147483648) // RAM disk size
		FILE_SIZE = 2147483648;

	strcpy(filename, argv[3]);
	c = filename[0];

	if (posix_memalign(&buf1, END_SIZE, END_SIZE)) { // up to 64MB
		printf("ERROR - POSIX NOMEM!\n");
		return 0;
	}

	buf = (char *)buf1;
	fd = open("/mnt/ramdisk/test1", O_CREAT | O_RDWR, 0640); 
	printf("fd: %d\n", fd);
	req_size = atoi(argv[1]);
	if (req_size > END_SIZE)
		req_size = END_SIZE;

	data = (char *)mmap(NULL, FILE_SIZE, PROT_WRITE, MAP_SHARED | MAP_POPULATE, fd, 0);
	size = req_size;
	memset(buf, c, size);
	lseek(fd, 0, SEEK_SET);
	count = FILE_SIZE / size;
	printf("Start c: %c\n", c);

	clock_gettime(CLOCK_MONOTONIC, &begin);
	for (i = 0; i < count; i++) {
        addr = data + size * i;
		memset(addr, c, size);
//		msync(data + size * i, size, MS_SYNC);
        flush_clwb(addr, size);
//        drain_pcommit();
	}
	clock_gettime(CLOCK_MONOTONIC, &finish);

	time1 = (finish.tv_sec * 1e9 + finish.tv_nsec) - (begin.tv_sec * 1e9 + begin.tv_nsec);
	printf("Mmap write(memset) %lld ns, average %lld ns\n", time1, time1 / count);

//	fsync(fd);
	close(fd);

	free(buf1);
	return 0;
}
