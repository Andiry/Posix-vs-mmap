#define _GNU_SOURCE

#include <sys/mman.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

#include <fcntl.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <time.h>
#include <sys/times.h>
#include <sys/file.h>
#include <sys/resource.h>
#include <sys/shm.h>

long long page_size = 4096;


static double time_so_far(void)
{
  struct timeval tp;

  if (gettimeofday(&tp, (struct timezone *) NULL) == -1)
    perror("gettimeofday");
	printf("Time: %f\n",((double) (tp.tv_sec)) +
    (((double) tp.tv_usec) * 0.000001 ));
  return ((double) (tp.tv_sec)) +
    (((double) tp.tv_usec) * 0.000001 );
}

/************************************************************************/
/* read_perf_test				        		*/
/* Read and re-fread test						*/
/************************************************************************/
void 
read_perf_test(unsigned long kilo64,long long reclen,long long *data1,long long *data2)
{
	double starttime2;
	double compute_val = (double)0;
	double readtime[2];
	double walltime[2], cputime[2];
	long long j;
	long long traj_size;
	unsigned long i,numrecs64,traj_offset;
	unsigned long lock_offset=0;
	long long Index = 0;
	unsigned long long readrate[2];
	unsigned long filebytes64;
	volatile char *buffer1;
	char *nbuff;
	char *mainbuffer;
	int fd,open_flags;
	int test_foo,ltest;
	long wval;
	double qtime_start,qtime_stop;
	double hist_time;
	unsigned long r_traj_ops_completed=0;
	unsigned long r_traj_bytes_completed=0;
	long long *gc=0;
	char *filename = "/mnt/ramdisk/test1";
	size_t len = reclen;
	struct timespec start, end;
	long long time;

	hist_time=qtime_start=qtime_stop=0;
	traj_offset=0;
	test_foo=0;

	numrecs64 = (kilo64*1024)/reclen;

	open_flags = O_RDONLY;
	
	filebytes64 = numrecs64*reclen;
	fd = 0;

	mainbuffer = (char *)malloc(4 * 1024 * 4096);
//	posix_memalign(&mainbuffer, (4 * 1024 * 4096), (4 * 1024 *4096));

	/* 
	 * begin real testing
	 */
	ltest=3;
printf("%s: filename %s, test %d, reclen %lld, numrecs64 %llu, filebytes64 %llu\n", __func__, filename, ltest, reclen, numrecs64, filebytes64);


//	fd = open(filename, open_flags,0640);

	for( j=0; j<ltest; j++ )
	{

//		lseek(fd,0,SEEK_SET);
		if((fd = open(filename, open_flags,0))<0)
		{
			printf("\nCan not open temporary file %s for read\n",filename);
			perror("open");
			exit(58);
		}

		printf("fd: %d, reclen %lu\n", fd, reclen);
		  fsync(fd);

		/* 
		 *  Need to prime the instruction cache & TLB
		 */
		nbuff=mainbuffer;
//		read(fd, (void *)nbuff, (size_t)reclen);
//		if(fetchon)
//			fetchit(nbuff,reclen);

		if(read(fd, (void *)nbuff, 4096) != reclen)
		{
#ifdef _64BIT_ARCH_
			printf("\nError reading block %d %llx\n", 0,
				(unsigned long long)nbuff);
#else
			printf("\nError reading block %d %lx\n", 0,
				(long)nbuff);
#endif
			perror("read");
			exit(60);
		}
		lseek(fd,0,SEEK_SET);

		nbuff=mainbuffer;

		starttime2 = time_so_far();

		compute_val=(double)0;
		r_traj_ops_completed=0;
		r_traj_bytes_completed=0;

		clock_gettime(CLOCK_MONOTONIC, &start);
		for(i=0; i<numrecs64; i++) 
		{
			    wval=read((int)fd, (void*)nbuff, 4096);
			    if(wval != reclen)
			    {
				printf("\nError reading block %lld %llx\n", i,
					(unsigned long long)nbuff);
				perror("read");
				exit(61);
			    }
		
		
			r_traj_ops_completed++;
			r_traj_bytes_completed+=reclen;
		}
		
		clock_gettime(CLOCK_MONOTONIC, &end);
		time = (end.tv_sec - start.tv_sec) * 1e9 + (end.tv_nsec - start.tv_nsec);
		printf("%lld ns\n", time);
		readtime[j] = ((time_so_far() - starttime2))-compute_val;
		if(readtime[j] < (double).000001) 
		{
			readtime[j]= (double)0.000001;
		}

		fsync(fd);
		close(fd);
	}

	filebytes64=r_traj_bytes_completed;
	printf("%s: completed %llu\n", __func__, filebytes64);

        for(j=0;j<ltest;j++)
        {
                  readrate[j] = 
                  (unsigned long long) ((double) filebytes64 / readtime[j]);
		  printf("%s: ltest %d readrate %llu\n", __func__, j, readrate[j]);
			
	}

	free(mainbuffer);
}

int main(void)
{
	read_perf_test(1048576, 4096, NULL, NULL);
	return 0;
}
