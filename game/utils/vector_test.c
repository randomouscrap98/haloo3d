#include "vector.h"
#include "test.h"

// Make sure all these weird ass macros at least produce
// valid code...
VECTOR_DECLARE(int);
VECTOR_DEFINE(int);

typedef struct {
  vector_int * v;
  size_t index;
} vintpointer;

void vector_test() {
  // Initialization
  vector_int v;
  ASSERT(vector_int_init(&v) == 0, "vector_int_init work");
  ASSERT(v.length == 0, "vector length 0");

  // Some inserts
  int val = 5;
  ASSERT(vector_int_push(&v, &val) == 0, "vector_int_push [0]");
  ASSERT(v.length == 1, "vector length 1");
  ASSERT(v.array[0] == 5, "vector[0] = 5");
  val = 7;
  ASSERT(vector_int_push(&v, &val) == 0, "vector_int_push [1]");
  ASSERT(v.length == 2, "vector length 2");
  ASSERT(v.array[1] == 7, "vector[1] = 7");

  // Bad reserves
  ASSERT(vector_int_reserve(&v, 2) != 0, "vector_int_reserve fail on 2");
  ASSERT(vector_int_reserve(&v, 1) != 0, "vector_int_reserve fail on 1");

  // Insert a huge pile of integers
  size_t olength = v.length;
  size_t ocapacity = v.capacity;
  for(int i = 2; i < 100; i++) {
    ASSERT(vector_int_push(&v, &i) == 0, "vector_int_push [%d]", i);
    ASSERT(v.length == olength + 1, "vector length + 1");
    ASSERT(v.capacity >= v.length, "vector capacity(%zu) >= length", v.capacity);
    ASSERT(v.capacity >= ocapacity, "vector capacity >= old");
    ASSERT(v.array[i] == i, "vector i = %d", i);
    olength = v.length;
    ocapacity = v.capacity;
  }

  ASSERT(vector_int_decrement(&v) == 0, "vector_int_decrement works");
  ASSERT(v.length == olength - 1, "vector length -1");
  size_t index;
  ASSERT(vector_int_increment(&v, &index) == 0, "vector_int_increment works");
  ASSERT(v.length == olength, "vector length back up");
  ASSERT(index == olength - 1, "vector increment index correct");

  // Clear it out
  vector_int_clear(&v);
  ASSERT(v.length == 0, "vector_int_clear length 0");

  vector_int_free(&v);
  ASSERT(1, "vector_int_free");
}
