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
#include"FastRand.hpp"
#include"NVTMWorkloadHarness.hpp"
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "btree.hpp"

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

void ParseOptions(int argv, char **argv);

struct thread_data {
	struct request *request;
	unsigned int num_request;
	unsigned int index;
};

void PopulateThreadData(unsigned long long op_count, struct thread_data *td)
{
	unsigned long long total_reads = op_count * rw_ratio/100;
	unsigned long long total_writes = (unsigned long long) (op_count *1.0 *(1 - (float) 1.0 * rw_ratio/100) );
	cout << " Op count " << op_count << " Total Reads " << total_reads << " Total Writes " << total_writes << endl;

//    uint64_t seed = rand();
	for(int j = 0; j < num_threads; j++){
		unsigned long long reads = total_reads / num_threads;
		unsigned long long writes = total_writes / num_threads; // each thread has a predefined set of unique request
		td[j].request = (struct request *) calloc((reads + writes), sizeof(struct request));      
		td[j].num_request = reads + writes; 
		td[j].index = 0;

		int k = 0;
		while(reads > 0 || writes > 0) {
			if(reads != 0) {
				td[j].request[k].rnw = 1; //read 
				td[j].request[k].offset = Aligned(seed, req_size);
				k++;
				reads--;
			} else if (writes != 0){
				td[j].request[k].rnw = 0;
				td[j].request[k].offset = Aligned(seed, req_size);
				k++;
				writes--;
			}
//			RandLFSR(&seed);
                }
	}                   
}

