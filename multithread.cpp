#include <iostream>
#include<string>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <signal.h>
#include <sys/resource.h>
#include <sys/uio.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <assert.h>
#include <limits.h>
#include <sstream>
#include <fstream>
#include "Util/FastRand.hpp"
#include "Util/NVTMWorkloadHarness.hpp"
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace std;

struct request{
	unsigned int rnw;
	uint64_t offset;
};

unsigned int rw_ratio = 100;
int num_threads;
size_t req_size = 4096;
int file_count = 1;
int *file_descriptors;
size_t workload_size = 1UL * 1024 * 1024 * 1024;

char **buf;

void ParseOptions(int argc, char **argv);

struct thread_data {
	struct request *request;
	unsigned int num_request;
	unsigned int index;
};

void PopulateThreadData(unsigned long long op_count, struct thread_data *td)
{
	unsigned long long total_reads = op_count * rw_ratio / 100;
	unsigned long long total_writes = (unsigned long long) (op_count *1.0 *(1 - (float) 1.0 * rw_ratio/100) );
	cout << " Op count " << op_count << " Total Reads " << total_reads << " Total Writes " << total_writes << endl;

//	uint64_t seed = rand();
	for(int j = 0; j < num_threads; j++){
		unsigned long long reads = total_reads / num_threads;
		unsigned long long writes = total_writes / num_threads; // each thread has a predefined set of unique request
		uint64_t offset = (reads + writes) * req_size * j;
		td[j].request = (struct request *) calloc((reads + writes), sizeof(struct request));      
		td[j].num_request = reads + writes; 
		td[j].index = 0;

		int k = 0;
		while(reads > 0 || writes > 0) {
			if(reads != 0) {
				td[j].request[k].rnw = 1; //read 
				td[j].request[k].offset = offset;
				k++;
				reads--;
			} else if (writes != 0) {
				td[j].request[k].rnw = 0;
				td[j].request[k].offset = offset;
				k++;
				writes--;
			}
			offset += req_size;
//			RandLFSR(&seed);
                }
	}                   
}


void FileOps(int id, void *arg, uint64_t &seed)
{
        struct thread_data *tds =(struct thread_data*) arg;
        struct thread_data *td = &(tds[id]);
        int fd = file_descriptors[id % file_count];
	int ret;

        if(td->request[td->index % td->num_request].rnw)
        {
		ret = pread(fd, buf[id], req_size, td->request[td->index % td->num_request].offset);
        }else{
		ret = pwrite(fd, buf[id], req_size, td->request[td->index %td->num_request].offset);
        }
	assert(ret == req_size);
        td->index++;
}

int main (int argc, char *argv[]) {
	stm::NVTMWorkloadHarness::Init("MultithreadTest", argc, argv);
	stm::NVTMWorkloadHarness::SuspendTiming();

	ParseOptions(argc, argv);
	workload_size = stm::NVTMWorkloadHarness::GetFootPrintBytes();
	if(workload_size < 0) { fprintf(stderr, "Illegal foot: %li\n", workload_size); exit(EXIT_FAILURE); }

	num_threads = stm::NVTMWorkloadHarness::GetThreadCount();

	if( (num_threads < 1) || (num_threads > 128) ) {
		fprintf(stderr, "Illegal thread count: %i\n", num_threads); exit(EXIT_FAILURE);
	}

#define DUMP(x) std::cout << #x << "=" << x << "\n" 

	DUMP(num_threads);
	DUMP(req_size);
	DUMP(file_count);
	DUMP(workload_size);
	DUMP(rw_ratio);
	if(rw_ratio == 0)
		cout << "Operation=write\n";
	else if(rw_ratio == 100)
		cout << "Operation=read\n";
	else 
		cout << "Operation=mixed\n";
  
	if (file_count > 1) {
		mkdir(stm::NVTMWorkloadHarness::GetHeapFileName().c_str(), 0777);
	}

	file_descriptors = new int[file_count];
	int fd;
	for (int i = 0; i < file_count; i++) {
		std::stringstream name;
		struct stat st;
		name << stm::NVTMWorkloadHarness::GetHeapFileName(); 

		cout << "Target=" << name.str().c_str() << "\n";
		if (stm::NVTMWorkloadHarness::GetCreateFlag()) {
			fd = open(name.str().c_str(), O_RDWR|O_CREAT|O_DIRECT, 0666);
		} else {
			fd = open(name.str().c_str(), O_RDWR|O_DIRECT, 0666);
		}
          
		assert(fd >= 0);
		file_descriptors[i] = fd;
	}

	buf = (char**) calloc(num_threads, sizeof(char*));
	struct thread_data *td = (struct thread_data*) calloc(num_threads, sizeof(struct thread_data));
	for (int i = 0; i < num_threads; i++) { 
		if (posix_memalign((void**)(buf+i), 512, req_size) != 0) {
			assert(0);
		}
	}

	unsigned long long op_count = workload_size / req_size;
	stm::NVTMWorkloadHarness::SetOperationCount(op_count);  
	std::cout << "Setting up thread data " << "\n";
	PopulateThreadData(op_count, td);       
	std::cout << "Setup of thread data complete op_count " << stm::NVTMWorkloadHarness::GetOperationCount() << "\n";
	// Removed all that boilerplate crap.  Now you just need the Op() function.

	for( int j = 0; j < num_threads; j++)
		td[j].index = 0;

	stm::NVTMWorkloadHarness::StartTiming();
	stm::NVTMWorkloadHarness::RunOps(FileOps, td, num_threads);

	stm::NVTMWorkloadHarness::StopTiming();
	stm::NVTMWorkloadHarness::PrintResults();
	for( int j = 0; j < num_threads; j++) {
		free(buf[j]);
		free(td[j].request);
	}
	free(buf);
	free(td);
	return 0;
}

void ParseOptions(int argc, char *argv[])
{
	char c;     

	// if it takes an argument you put a ':' after it.  Otherwise, no arg.
	while ((c = getopt(argc, argv, "r:s:F:R:W:C:B:T:")) != -1) {
		switch (c) {
		case 'r':
			rw_ratio = atoi(optarg);
			if( (rw_ratio < 0) || (rw_ratio > 100) ) { fprintf(stderr, "Illegal ratio: %i\n", rw_ratio); exit(EXIT_FAILURE); }
			break;
		case 's':
			req_size = atoll(optarg);
			if(req_size < 0) { fprintf(stderr, "Illegal request size: %li\n", req_size); exit(EXIT_FAILURE); }
			break;
		case 'F':
			file_count = atoi(optarg);
			break;
		default:
			std::cerr << "Usage: " << argv[0]  << " " << stm::NVTMWorkloadHarness::GetStandardOptionsUsage() << " -r <read ratio 0-100> -s <req size in bytes> -F <File count>\n";
			exit(EXIT_FAILURE);
		}
	}
}
