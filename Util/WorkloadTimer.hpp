#ifndef STM_WORKLOAD_TIMER_INCLUDED
#define STM_WORKLOAD_TIMER_INCLUDED

#include <unistd.h>
#include <signal.h>

namespace NVSL {

     class WorkloadTimer {
	  
     public:
	  WorkloadTimer(int seconds, 
			void(*callback)(int)) {
	       struct sigaction act;
	       bzero(&act, sizeof(act));
	       act.sa_handler = callback;
	       act.sa_flags = SA_RESETHAND;
	       sigaction(SIGALRM, &act, NULL);
	       alarm(seconds);
	  }
     };
}
#endif

