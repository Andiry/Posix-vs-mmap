#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define FUNC(x,y) \
{ \
  (x) ^= (y); \
  (y) ^= (x); \
  (x) ^= (y); \
}

int main(int argc, char **argv) {
  srand48(0);

  const size_t n = 1000;
  int *tab = (int*)malloc(n * sizeof(int));
  for (int i = 0; i < n; ++i) tab[i] = i;

  for (int i = 0; i < n; ++i) {
    int i1 = (int)ceil(n * drand48());
    int i2 = (int)ceil(n * drand48());
    FUNC(tab[i1], tab[i2]);
  }

  for (int i = 0; i < n; ++i) printf("%d ", tab[i]);
  printf("\n");
  return 0;
}





