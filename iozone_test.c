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
char *mainbuffer;
off64_t offset64 = 0;

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

//	mainbuffer = (char *)malloc(4 * 1024 * 4096);

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

}

void 
write_perf_test(unsigned long kilo64,long long reclen,long long *data1,long long *data2)
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

	open_flags = O_RDWR | O_CREAT;
	
	filebytes64 = numrecs64*reclen;
	fd = 0;

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

		if(write(fd, (void *)nbuff, 4096) != reclen)
		{
#ifdef _64BIT_ARCH_
			printf("\nError writing block %d %llx\n", 0,
				(unsigned long long)nbuff);
#else
			printf("\nError writing block %d %lx\n", 0,
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
			    wval=write((int)fd, (void*)nbuff, 4096);
			    if(wval != reclen)
			    {
				printf("\nError writing block %lld %llx\n", i,
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
		  printf("%s: ltest %d writerate %llu\n", __func__, j, readrate[j]);
			
	}

}

/************************************************************************/
/* write_perf_test ()				        		*/
/* Write and re-write test						*/
/************************************************************************/
void write_perf_test1(off64_t kilo64,long long reclen ,long long *data1,long long *data2)
{
	double starttime1;
	double writetime[2];
	double walltime[2], cputime[2];
	double qtime_start,qtime_stop;
	double hist_time;
	double compute_val = (double)0;
	long long i,j;
	off64_t numrecs64,traj_offset;
	off64_t lock_offset=0;
	long long Index = 0;
	long long file_flags = 0;
	long long traj_size;
	unsigned long long writerate[2];
	off64_t filebytes64;
	int ltest;
	char *maddr;
	char *wmaddr,*free_addr;
	char *pbuff;
	char *nbuff;
	int fd,wval;
	int notruncate = 1;
	unsigned long w_traj_ops_completed=0;
	unsigned long w_traj_bytes_completed=0;
	char *filename = "/mnt/ramdisk/test1";

	int test_foo;

	nbuff=wmaddr=free_addr=0;
	traj_offset=0;
	test_foo=0;
	hist_time=qtime_start=qtime_stop=0;
	maddr=0;
	pbuff=mainbuffer;
	numrecs64 = (kilo64*1024)/reclen;
	filebytes64 = numrecs64*reclen;

	fd = 0;
	file_flags = O_RDWR;

/* Sanity check */
/* Some filesystems do not behave correctly and fail
 * when this sequence is performned. This is a very
 * bad thing. It breaks many applications and lurks
 * around quietly. This code should never get
 * triggered, but in the case of running iozone on
 * an NFS client, the filesystem type on the server
 * that is being exported can cause this failure.
 * If this failure happens, then the NFS client is
 * going to going to have problems, but the acutal
 * problem is the filesystem on the NFS server.
 * It's not NFS, it's the local filesystem on the
 * NFS server that is not correctly permitting
 * the sequence to function.
 */
/* _SUA_ Services for Unix Applications, under Windows
    does not have a truncate, so this must be skipped */
        if((fd = open(filename, (int)O_CREAT|O_RDWR,0))<0)
        {
                printf("\nCan not open temp file: %s\n",
                        filename);
                perror("open");
                exit(44);
        }
		if(!notruncate)
		{
			wval=ftruncate(fd,0);
			if(wval < 0)
			{
				printf("\n\nSanity check failed. Do not deploy this filesystem in a production environment !\n");
				exit(44);
			}
			close(fd);

		}
/* Sanity check */

	ltest=3;

	for( j=0; j<ltest; j++)
	{
		if(j==0)
		{
			if(!notruncate)
			{
	  	   		if((fd = creat(filename, 0640))<0)
	  	   		{
					printf("\nCan not create temp file: %s\n", 
						filename);
					perror("creat");
					exit(42);
	  	   		}
			}
		}
		if(fd) 
			close(fd);

	  	 if((fd = open(filename, (int)file_flags,0))<0)
	  	 {
			printf("\nCan not open temp file: %s\n", 
				filename);
			perror("open");
			exit(44);
	  	 }

		wval=fsync(fd);
		if(wval==-1){
			perror("fsync");
		}
		pbuff=mainbuffer;
		starttime1 = time_so_far();

		compute_val=(double)0;
		w_traj_ops_completed=0;
		w_traj_bytes_completed=0;

		for(i=0; i<numrecs64; i++){
			    wval=write(fd, pbuff, (size_t ) reclen);
			    if(wval != reclen)
			    {
#ifdef NO_PRINT_LLD
			    	printf("\nError writing block %ld, fd= %d\n", i,
					 fd);
#else
			    	printf("\nError writing block %lld, fd= %d\n", i,
					 fd);
#endif
			    	if(wval == -1)
					perror("write");
			    }
			w_traj_ops_completed++;
			w_traj_bytes_completed+=reclen;
		}

		writetime[j] = ((time_so_far() - starttime1))
			-compute_val;
		if(writetime[j] < (double).000001) 
		{
			writetime[j]=(double).000001;
		}
		wval=close(fd);
		if(wval==-1){
			perror("close");
		}
	}

	filebytes64=w_traj_bytes_completed;
		
        for(j=0;j<ltest;j++)
        {
                  writerate[j] = 
                    (unsigned long long) ((double) filebytes64 / writetime[j]);
		  printf("%s: ltest %d writerate %llu\n", __func__, j, writerate[j]);
	}

}

/************************************************************************/
/* random_perf_test				        		*/
/* Random read and write test						*/
/************************************************************************/
void random_perf_test(off64_t kilo64,long long reclen,long long *data1,long long *data2)
{
	double randreadtime[2];
	double starttime2;
	double walltime[2], cputime[2];
	double compute_val = (double)0;
	unsigned long long big_rand;
	long long j;
	off64_t i,numrecs64;
	long long Index=0;
	int flags;
	unsigned long long randreadrate[2];
	off64_t filebytes64;
	off64_t lock_offset=0;
	volatile char *buffer1;
	char *wmaddr,*nbuff;
	char *maddr,*free_addr;
	int fd,wval;
	long long *recnum= 0;
	long long *gc=0;
	char *filename = "/mnt/ramdisk/test1";

	maddr=free_addr=0;
	numrecs64 = (kilo64*1024)/reclen;
        srand48(0);

        recnum = (long long *)malloc(sizeof(*recnum)*numrecs64);
        if (recnum){
             /* pre-compute random sequence based on 
		Fischer-Yates (Knuth) card shuffle */
            for(i = 0; i < numrecs64; i++){
                recnum[i] = i;
            }
            for(i = 0; i < numrecs64; i++) {
                long long tmp;
               big_rand = lrand48();
               big_rand = big_rand%numrecs64;
               tmp = recnum[i];
               recnum[i] = recnum[big_rand];
               recnum[big_rand] = tmp;
            }
        }
	else
	{
		fprintf(stderr,"Random uniqueness fallback.\n");
	}
	flags = O_RDWR;

	fd=0;
	filebytes64 = numrecs64*reclen;
	for( j=0; j<2; j++ )
	{
		if(j==0)
			flags |=O_CREAT;
	     if((fd = open(filename, ((int)flags),0640))<0){
			printf("\nCan not open temporary file for read/write\n");
			perror("open");
			exit(66);
	     }

	     nbuff=mainbuffer;
             srand48(0);

	     compute_val=(double)0;
	     starttime2 = time_so_far();
	     if ( j==0 ){
		for(i=0; i<numrecs64; i++) {
                        if (recnum) {
				offset64 = reclen * (long long)recnum[i];
                        }
			else
			{
			   offset64 = reclen * (lrand48()%numrecs64);
			}

			   if(lseek( fd, offset64, SEEK_SET )<0)
			   {
				perror("lseek");
				exit(68);
			   }
		  	
			     if(read(fd, (void *)nbuff, (size_t)reclen) != reclen)
		  	     {
#ifdef NO_PRINT_LLD
				 printf("\nError reading block at %ld\n",
					 offset64); 
#else
				 printf("\nError reading block at %lld\n",
					 offset64); 
#endif
				 perror("read");
				 exit(70);
		 	     }
		}
	     }
	     else
	     {
			for(i=0; i<numrecs64; i++) 
			{
                                if (recnum) {
				  offset64 = reclen * (long long)recnum[i];
                                }
			        else
			        {
				  offset64 = reclen * (lrand48()%numrecs64);
				}

				  lseek( fd, offset64, SEEK_SET );
			 	  wval=write(fd, nbuff,(size_t)reclen);
			  	  if(wval != reclen)
			  	  {
#ifdef NO_PRINT_LLD
					printf("\nError writing block at %ld\n",
						offset64); 
#else
					printf("\nError writing block at %lld\n",
						offset64); 
#endif
					if(wval==-1)
						perror("write");
		 		  }
			}
	     } 	/* end of modifications	*kcollins:2-5-96 */
	        randreadtime[j] = ((time_so_far() - starttime2))-
			compute_val;
	     if(randreadtime[j] < (double).000001) 
	     {
			randreadtime[j]=(double).000001;
	     }
	     		wval=fsync(fd);
			if(wval==-1){
				perror("fsync");
//				signal_handler();
			}
		wval=close(fd);
		if(wval==-1){
			perror("close");
//			signal_handler();
		}
    	}
        for(j=0;j<2;j++)
        {
                  randreadrate[j] = 
		      (unsigned long long) ((double) filebytes64 / randreadtime[j]);
		  printf("%s: ltest %d randomrate %llu\n", __func__, j, randreadrate[j]);
	}
	/* Must save walltime & cputime before calling store_value() for each/any cell.*/
	if(recnum)
		free(recnum);
}


int main(void)
{
//	mainbuffer = (char *)malloc(4 * 1024 * 4096);
	posix_memalign(&mainbuffer, (4 * 1024 * 4096), (4 * 1024 *4096));
	write_perf_test1(1048576, 4096, NULL, NULL);
	read_perf_test(1048576, 4096, NULL, NULL);
	random_perf_test(1048576, 4096, NULL, NULL);
	free(mainbuffer);
	return 0;
}
