#ifndef STM_ATOMIC_OPS_INCLUDED
#define STM_ATOMIC_OPS_INCLUDED
#include<stdint.h>
#include <boost/type_traits.hpp>

namespace stm {
     inline int atomic_exchange_and_add( int * pw, int dv )
     {
	  // int r = *pw;
	  // *pw += dv;
	  // return r;

	  int r;

	  __asm__ __volatile__
	       (
		"lock\n\t"
		"xadd %1, %0":
		"=m"( *pw ), "=r"( r ): // outputs (%0, %1)
		"m"( *pw ), "1"( dv ): // inputs (%2, %3 == %1)
		"memory", "cc" // clobbers
		);

	  return r;
     }

     inline unsigned int atomic_exchange_and_add( unsigned int * pw, unsigned int dv )
     {
	  // unsigned int r = *pw;
	  // *pw += dv;
	  // return r;

	  int r;

	  __asm__ __volatile__
	       (
		"lock\n\t"
		"xadd %1, %0":
		"=m"( *pw ), "=r"( r ): // outputs (%0, %1)
		"m"( *pw ), "1"( dv ): // inputs (%2, %3 == %1)
		"memory", "cc" // clobbers
		);

	  return r;
     }

     inline long long atomic_exchange_and_add( long long * pw, long long dv )
     {
	  // int r = *pw;
	  // *pw += dv;
	  // return r;

	  long long r;

	  __asm__ __volatile__
	       (
		"lock\n\t"
		"xaddq %1, %0":
		"=m"( *pw ), "=r"( r ): // outputs (%0, %1)
		"m"( *pw ), "1"( dv ): // inputs (%2, %3 == %1)
		"memory", "cc" // clobbers
		);

	  return r;
     }


     inline int atomic_exchange_and_add( volatile int * pw, int dv )
     {
	  // int r = *pw;
	  // *pw += dv;
	  // return r;

	  int r;

	  __asm__ __volatile__
	       (
		"lock\n\t"
		"xadd %1, %0":
		"=m"( *pw ), "=r"( r ): // outputs (%0, %1)
		"m"( *pw ), "1"( dv ): // inputs (%2, %3 == %1)
		"memory", "cc" // clobbers
		);

	  return r;
     }
     inline long long atomic_exchange_and_add( volatile long long * pw, long long dv )
     {
	  // int r = *pw;
	  // *pw += dv;
	  // return r;

	  long long r;

	  __asm__ __volatile__
	       (
		"lock\n\t"
		"xaddq %1, %0":
		"=m"( *pw ), "=r"( r ): // outputs (%0, %1)
		"m"( *pw ), "1"( dv ): // inputs (%2, %3 == %1)
		"memory", "cc" // clobbers
		);

	  return r;
     }



     inline void MemoryBarrier() {
  	  __asm__ __volatile__
  	       (
  		"mfence\n\t"
		:::
		"memory"
  		);
     }


     template<typename T>
     inline T atomic_exchange( volatile T * pw, T dv )
     {
	  
	  // int r = *pw;
	  // *pw = dv;
	  // return r;

	  int r;

	  __asm__ __volatile__
	       (
		"xchg %1, %0":
		"=m"( *pw ), "=r"( r ): // outputs (%0, %1)
		"m"( *pw ), "1"( dv ): // inputs (%2, %3 == %1)
		"memory"// clobbers
		);

	  return r;
     }


     template<typename T>
     inline T atomic_compare_and_exchange(T requiredOldValue, volatile T * _ptr, T newValue,  const boost::integral_constant<int, 8>& )
     {
#define ll_low(x)       *(((unsigned int*)&(x))+0)
#define ll_high(x)      *(((unsigned int*)&(x))+1)

	  T old;
	  asm volatile(
			       "mfence\n\t"
			       "mov            %5, %%ecx  \n\t"
			       "mov            %4, %%ebx  \n\t"
			       "mov            %6, %%eax  \n\t"
			       "mov            %7, %%edx  \n\t"
			       "lock\n\t"
			       "cmpxchg8b %2\n\t"
			       "mov            %%eax, %0\n\t"
			       "mov            %%edx, %1\n\t"
			       "mov            %%edx, %1\n\t"
			       "mfence\n\t"
			       : "=m" (ll_low(old)), "=m" (ll_high(old)), "=m" (*_ptr)
			       : "m" (*_ptr), "m" (ll_low(newValue)), "m" (ll_high(newValue)), "m" (ll_low(requiredOldValue)), "m" (ll_high(requiredOldValue))
			       : "memory", "eax", "ebx", "ecx", "edx", "cc");
	  // you cannot swap 64 bit values on 32 bit machine with this code.    you need to find a way to spill and restore ebx
	  return old;

     }

     template<typename T>
     inline T atomic_compare_and_exchange(T requiredOldValue, volatile T * _ptr, T newValue, const boost::integral_constant<int, 4>& )
     {
	  T old;
	  
  	  asm volatile
  	       (
		"mov %3, %%eax;\n\t"
  	       	"lock\n\t"
  		"cmpxchg %4, %0\n\t"
  		"mov %%eax, %1\n\t"
		:
  		"=m" ( *_ptr ), "=r" ( old  ): // outputs (%0 %1)
  		"m" ( *_ptr ), "r" ( requiredOldValue), "r" ( newValue ): // inputs (%2, %3, %4)
  		"memory", "eax", "cc" // clobbers
  		);

  	  return old;
     }
     
     template<typename T>
     inline T atomic_compare_and_exchange(T requiredOldValue, volatile T * _ptr, T newValue )
     {
	  return atomic_compare_and_exchange(requiredOldValue, _ptr, newValue, boost::integral_constant<int, sizeof(T)>());
     }

     template<class T>
     inline T* atomic_compare_and_exchange_ptr(T * requiredOldValue, T *volatile *_ptr, T * newValue )
     {
	  return atomic_compare_and_exchange(requiredOldValue, _ptr, newValue);
     }
     

     inline void atomic_increment(int *pw)
     {
	  asm (
	       "lock\n\t"
	       "incl %0":
	       "=m"(*pw): // output (%0)
	       "m"(*pw): // input (%1)
	       "cc" // clobbers
	       );
     }

     inline void atomic_increment(unsigned int *pw)
     {
	  asm (
	       "lock\n\t"
	       "incl %0":
	       "=m"(*pw): // output (%0)
	       "m"(*pw): // input (%1)
	       "cc" // clobbers
	       );
     }

     inline void atomic_increment(long long *pw)
     {
	  asm (
	       "lock\n\t"
	       "incq %0":
	       "=m"(*pw): // output (%0)
	       "m"(*pw): // input (%1)
	       "cc" // clobbers
	       );
     }

     inline void atomic_increment(volatile int *pw)
     {
	  asm (
	       "lock\n\t"
	       "incl %0":
	       "=m"(*pw): // output (%0)
	       "m"(*pw): // input (%1)
	       "cc" // clobbers
	       );
     }

     inline void atomic_increment(volatile long long *pw)
     {
	  asm (
	       "lock\n\t"
	       "incq %0":
	       "=m"(*pw): // output (%0)
	       "m"(*pw): // input (%1)
	       "cc" // clobbers
	       );
     }

     inline void atomic_decrement(int *pw)
     {
	  asm (
	       "lock\n\t"
	       "decl %0":
	       "=m"(*pw): // output (%0)
	       "m"(*pw): // input (%1)
	       "cc" // clobbers
	       );
     }
}

#endif
