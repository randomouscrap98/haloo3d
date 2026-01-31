#ifndef __UTILS_UTILS_H
#define __UTILS_UTILS_H

#include <stdlib.h>

#define NULLFREE(x) \
  if(x != NULL) { \
    free(x); \
    x = NULL; \
  }

#endif
