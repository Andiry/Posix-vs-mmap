#define _GNU_SOURCE

#include<stdio.h>
#include<fcntl.h>
#include<unistd.h>
#include<signal.h>
#include<time.h>
#include<string.h>
#include<malloc.h>
#include<stdlib.h>
#include<sys/mman.h>
#include <sys/ioctl.h>
#include<sys/time.h>

#define END_SIZE	(4UL * 1024 * 1024)
#define	NOVA_RESTORE_MMAP_WRITE		0xBCD00019

const int start_size = 512;
unsigned long long global_file_size;
int global_fd;
char *global_addr;

static void handler(int sig, siginfo_t *si, void *unused)
{
	unsigned long addr;
	addr = (unsigned long)si->si_addr;

	ioctl(global_fd, NOVA_RESTORE_MMAP_WRITE, &addr);
	printf("SIGSEGV at address 0x%lx\n", addr);
}

static void setup_handler(void)
{
	struct sigaction sa;

	sa.sa_flags = SA_SIGINFO;
	sigemptyset(&sa.sa_mask);
	sa.sa_sigaction = handler;
	sigaction(SIGSEGV, &sa, NULL);
}

int main(int argc, char **argv)
{
	int fd, i;
	unsigned long long FILE_SIZE;
	unsigned long bufsize;
	size_t len, offset;
	char c;
	char unit;
	int size;
	unsigned long long count;
	void *buf1 = NULL;
	char *buf, *data;
	char file_size_num[20];
	char filename[60];
	int req_size;
	struct timespec begin, finish;
	long long time1;

	if (argc < 4) {
		printf("Usage: ./mmap_read $REQ_SIZE $FILE_SIZE $filename\n");
		return 0;
	}

	setup_handler();

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

	bufsize = END_SIZE;
	if (bufsize > FILE_SIZE)
		bufsize = FILE_SIZE;

	if (posix_memalign(&buf1, bufsize, bufsize)) { // up to 64MB
		printf("ERROR - POSIX NOMEM!\n");
		return 0;
	}

	buf = (char *)buf1;
	memset(buf, 'a', bufsize);
	fd = open("/mnt/ramdisk/test1", O_CREAT | O_RDWR, 0640); 
	printf("fd: %d\n", fd);
	count = FILE_SIZE / bufsize;
	if (count == 0)
		count++;

	for (i = 0; i < count; i++) {
		write(fd, buf, bufsize);
	}

	req_size = atoi(argv[1]);
	if (req_size > END_SIZE)
		req_size = END_SIZE;

	offset = 4096;
	data = (char *)mmap(NULL, FILE_SIZE - offset, PROT_READ | PROT_WRITE, MAP_SHARED, fd, offset);
	size = req_size;
	memset(buf, c, size);
	lseek(fd, 0, SEEK_SET);
	count = FILE_SIZE / size;
	printf("Start c: %c\n", c);
	printf("addr: %p\n", data);
	printf("count: %d\n", count);
	global_file_size = FILE_SIZE;
	global_fd = fd;
	global_addr = data;

	clock_gettime(CLOCK_MONOTONIC, &begin);
	for (i = 0; i < count - 1; i++) {
		memcpy(data + size * i, buf, size);
	}
	clock_gettime(CLOCK_MONOTONIC, &finish);

	time1 = (finish.tv_sec * 1e9 + finish.tv_nsec) - (begin.tv_sec * 1e9 + begin.tv_nsec);
	printf("Mmap write(memcpy) %lld ns, average %lld ns\n", time1, time1 / count);

	clock_gettime(CLOCK_MONOTONIC, &begin);
	msync(data, FILE_SIZE, MS_SYNC);
	clock_gettime(CLOCK_MONOTONIC, &finish);
	time1 = (finish.tv_sec * 1e9 + finish.tv_nsec) - (begin.tv_sec * 1e9 + begin.tv_nsec);
	printf("Msync %lld ns, average %lld ns\n", time1, time1 / count);

	printf("Sleep for 10 seconds..\n");
//	mprotect(data, FILE_SIZE, PROT_READ);

	sleep(10);

	printf("Sleep done.\n");
	memset(buf, 'z', size);
	clock_gettime(CLOCK_MONOTONIC, &begin);
	for (i = 2; i < count - 1; i++) {
		memcpy(data + size * i, buf, size);
	}
	clock_gettime(CLOCK_MONOTONIC, &finish);

//	fsync(fd);
	close(fd);

	free(buf1);
	return 0;
}
