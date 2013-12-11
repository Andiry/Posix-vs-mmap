#ifndef NVTM_FAST_RAND_INCLUDED
#define NVTM_FAST_RAND_INCLUDED

#include<stdint.h>
#include<stdlib.h>

#define TAP(a) (((a) == 0) ? 0 : ((1ull) << (((uint64_t)(a)) - (1ull))))

#define RAND_LFSR_DECL(BITS, T1, T2, T3, T4)				\
     inline static uint##BITS##_t RandLFSR##BITS(uint##BITS##_t *seed) {	\
	  if (*seed == 0) {						\
	       *seed = rand();						\
	  }								\
	  								\
	  const uint##BITS##_t mask = TAP(T1) | TAP(T2) | TAP(T3) | TAP(T4); \
	  *seed = (*seed >> 1) ^ (uint##BITS##_t)(-(*seed & (uint##BITS##_t)(1)) & mask); \
	  return *seed;							\
     }

RAND_LFSR_DECL(64, 64,63,61,60);
RAND_LFSR_DECL(32, 32,30,26,25);
RAND_LFSR_DECL(16, 16,14,13,11);
RAND_LFSR_DECL(8 ,  8, 6, 5, 4);

inline static uint64_t RandLFSR(uint64_t * x) {
     return RandLFSR64(x);
}

#endif
