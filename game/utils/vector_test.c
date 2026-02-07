#include "vector.h"
#include "test.h"

// Make sure all these weird ass macros at least produce
// valid code...
VECTOR_DECLARE(int);
VECTOR_DEFINE(int);

void linkedlist_test() {
  vector_int v;
  ASSERT(vector_int_init(&v) == 0, "vector_int_init work");
}
