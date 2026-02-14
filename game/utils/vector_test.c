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
  int val = 5, out;
  ASSERT(vector_int_push(&v, &val) == 0, "vector_int_push [0]");
  ASSERT(v.length == 1, "vector length 1");
  ASSERT(vector_int_get(&v, 0, &out) == 0, "vector_int_get[0] works");
  ASSERT(out == 5, "vector_int_get(0) = 5");
  val = 7;
  ASSERT(vector_int_push(&v, &val) == 0, "vector_int_push [1]");
  ASSERT(v.length == 2, "vector length 2");
  ASSERT(v.array[1] == 7, "vector[1] = 7");

  // decrement/increment
  ASSERT(vector_int_decrement(&v) == 0, "vector_int_decrement works");
  ASSERT(v.length == 1, "vector length 1");
  size_t index;
  ASSERT(vector_int_increment(&v, &index) == 0, "vector_int_increment works");
  ASSERT(v.length == 2, "vector length back up");
  ASSERT(index == 1, "vector increment index correct");

  val = 88;
  ASSERT(vector_int_set(&v, 0, &val) == 0, "vector_int_set[0] works");
  ASSERT(vector_int_get(&v, 0, &out) == 0, "vector_int_get[0] works");
  ASSERT(out == 88, "vector_int_get(0) = 88");
  val = 99;
  ASSERT(vector_int_set(&v, 1, &val) == 0, "vector_int_set[1] works");
  ASSERT(vector_int_get(&v, 1, &out) == 0, "vector_int_get[1] works");
  ASSERT(out == 99, "vector_int_get(1) = 99");

  // First/last
  ASSERT(vector_int_first(&v, &out) == 0, "vector_first works");
  ASSERT(out == 88, "vector_first is %d", out);
  ASSERT(vector_int_last(&v, &out) == 0, "vector_lastworks");
  ASSERT(out == 99, "vector_last is %d", out);

  // Bad reserves (we don't check for errors here on too small anymore)
  //ASSERT(vector_int_reserve(&v, 2) != 0, "vector_int_reserve fail on 2");
  //ASSERT(vector_int_reserve(&v, 1) != 0, "vector_int_reserve fail on 1");

  // Insert a huge pile of integers into a new vector
  vector_int v2;
  ASSERT(vector_int_init(&v2) == 0, "vector_init works again");

  size_t olength = v2.length;
  size_t ocapacity = v2.capacity;
  for(int i = 0; i < 100; i++) {
    ASSERT(vector_int_push(&v2, &i) == 0, "vector_int_push [%d]", i);
    ASSERT(v2.length == olength + 1, "vector length + 1");
    ASSERT(v2.capacity >= v2.length, "vector capacity(%zu) >= length", v.capacity);
    ASSERT(v2.capacity >= ocapacity, "vector capacity >= old");
    ASSERT(v2.array[i] == i, "vector i = %d", i);
    olength = v2.length;
    ocapacity = v2.capacity;
  }

  // Merge the two vectors
  ASSERT(vector_int_append(&v, &v2) == 0, "vector_mergeinto works");
  ASSERT(v.length == 102, "vector merge has correct length (%zu vs 102)", v.length);

  ASSERT(v.array[0] == 88, "vector 0 = %d", v.array[0]);
  ASSERT(v.array[1] == 99, "vector 1 = %d", v.array[1]);

  // Make sure all the values are correct
  for(int i = 2; i < 102; i++) {
    ASSERT(v.array[i] == i - 2, "vector i = %d", i);
  }

  ASSERT(vector_int_append_range(&v, &v2, 5, 7) == 0, "vector_append_range works");
  ASSERT(v.length == 104, "vector merge has correct length (%zu vs 102)", v.length);
  ASSERT(v.array[102] == 5, "vector 102 = %d", v.array[0]);
  ASSERT(v.array[103] == 6, "vector 103 = %d", v.array[1]);

  // Clear it out
  vector_int_clear(&v);
  ASSERT(v.length == 0, "vector_int_clear length 0");

  // Some funny tests after finding edge cases: you SHOULD be able to append an empty vector to another
  size_t oldlen = v2.length;
  size_t oldcap = v2.capacity;
  ASSERT(vector_int_append(&v2, &v) == 0, "append empty vector");
  ASSERT(oldlen == v2.length, "append empty vector no length change");
  ASSERT(oldcap == v2.capacity, "append empty vector no capacity change");
  ASSERT(vector_int_append_range(&v2, &v, 0, 1) == 1, "append empty vector out of range end");
  ASSERT(vector_int_append_range(&v2, &v, 1, 0) == 1, "append empty vector out of range start");

  vector_int_free(&v);
  ASSERT(1, "vector_int_free");
  vector_int_free(&v2);
  ASSERT(1, "vector_int_free v2");
}
