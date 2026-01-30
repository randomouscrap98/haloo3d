#ifndef __TEST_H__
#define __TEST_H__

#include <stdlib.h>
#include <stdio.h>

// Assert a truthy value
#define ASSERT(x, ...) { \
  fprintf(stderr, "TEST: " __VA_ARGS__); \
  fprintf(stderr, "\n"); \
  if(!(x)) { \
    printf("ERR: " __VA_ARGS__); \
    printf("\n"); \
    exit(1); \
  } \
}

#endif
