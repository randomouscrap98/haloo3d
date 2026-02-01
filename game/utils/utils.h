#ifndef __UTILS_UTILS_H
#define __UTILS_UTILS_H

#include <stdlib.h>

#define NULLFREE(x) \
  if(x != NULL) { \
    free(x); \
    x = NULL; \
  }

// Modify x (in place) to be the next power of 2 (32 bit)
#define NEXTPOW2_32(x) { \
  x--; \
  x |= x >> 1; \
  x |= x >> 2; \
  x |= x >> 4; \
  x |= x >> 8; \
  x |= x >> 16; \
  x++; \
}

// Give x aligned to y (VERY not optimized, unless you trust the compiler)
#define ALIGN(x,y) ((x) % (y) ? ((x) - ((x) % (y))) + (y) : (x))
// Give x div y rounded up to nearest whole (assume x and y are ints).
// VERY not optimized, unless you trust the compiler
#define DIVROUNDUP(x,y) (((x) / (y)) + ((x) % (y) ? 1 : 0))

#endif
