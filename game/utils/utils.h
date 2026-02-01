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

#endif
