#define _GNU_SOURCE

#include<stdio.h>
#include<fcntl.h>
#include<unistd.h>
#include<time.h>
#include<string.h>
#include<malloc.h>
#include<stdlib.h>
#include<sys/mman.h>
#include<sys/time.h>

#define END_SIZE	(4UL * 1024 * 1024) 

const int start_size = 512;

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
	char *buf, *buf2;
	FILE *output;
	char fs_type[20];
	char quill_enabled[40];
	char file_size_num[20];
	char filename[60];
	int req_size;

	if (argc < 6) {
		printf("Usage: ./integrity_test2_write $FS $SCNEARIO $REQ_SIZE $FILE_SIZE $filename\n");
		return 0;
	}

	strcpy(fs_type, argv[1]);
	strcpy(quill_enabled, argv[2]);

	strcpy(file_size_num, argv[4]);
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

	strcpy(filename, argv[5]);
	c = filename[0];
	output = fopen(filename, "a");

	if (posix_memalign(&buf1, END_SIZE, END_SIZE)) { // up to 64MB
		printf("ERROR - POSIX NOMEM!\n");
		return 0;
	}

	buf = (char *)buf1;
	buf2 = malloc(END_SIZE);
	fd = open("/mnt/ramdisk/test1", O_CREAT | O_RDWR | O_DIRECT, 0640); 
//	fd = open("/dev/null", O_WRONLY, 0640); 
//	fd = open("/dev/zero", O_RDONLY, 0640); 
	printf("fd: %d\n", fd);
	req_size = atoi(argv[3]);
	if (req_size > END_SIZE)
		req_size = END_SIZE;

	size = req_size;
	memset(buf, c, size);
	lseek(fd, 0, SEEK_SET);
	count = FILE_SIZE / size;
	printf("Start c: %c\n", c);
	for (i = 0; i < count; i++) {
		if (write(fd, buf, size) != size)
			printf("ERROR\n");
		c++;
		if (c > 'z')
			c = 'A';
		memset(buf, c, size);
	}

	fsync(fd);
	close(fd);

	fclose(output);
	free(buf1);
	free(buf2);
	return 0;
}
