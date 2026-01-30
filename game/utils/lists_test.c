#include "lists.h"
#include "test.h"

typedef struct {
  int order;
  float value;
} sortstruct;

#define SORTSTRUCT_COMPARE(a, b) ((a).order < (b).order)

void lists_test() {
  cbuffer cb;

  ASSERT(!cbuffer_init(&cb, sizeof(float), 100), "cbuffer_init");
  ASSERT(cb.element_size == 4, "cbuffer_init_element_size");
  ASSERT(cb.capacity == 100, "cbuffer_init_capacity");
  ASSERT(cb.head == 0, "cbuffer_init_head");
  ASSERT(cb.length == 0, "cbuffer_init_length");

  ASSERT(cbuffer_get(&cb, 100) == NULL, "cbuffer_get_oob");
  ASSERT(cbuffer_get(&cb, 999) == NULL, "cbuffer_get_oob_big");
  // Now make sure EVERYTHING empty
  for(int i = 0; i < 100; i++) {
    ASSERT(cbuffer_get(&cb, i) == NULL, "cbuffer_get_%d", i);
  }

  float ftest = 1.5;
  float * fget;
  ASSERT(!cbuffer_push(&cb, (void *)&ftest), "cbuffer_push");
  ASSERT(cb.length == 1, "cbuffer_push_length");
  fget = (float *)cbuffer_get(&cb, 0);
  ASSERT(*fget == ftest, "cbuffer_push");
  ASSERT(cbuffer_tail(&cb) == 0, "cbuffer_push_tail");

  ftest = 84930;
  ASSERT(!cbuffer_push(&cb, (void *)&ftest), "cbuffer_pushagain");
  ASSERT(cb.length == 2, "cbuffer_pushagain_length");
  fget = (float *)cbuffer_get(&cb, 0);
  ASSERT(*fget == 1.5, "cbuffer_pushagain_0");
  fget = (float *)cbuffer_get(&cb, 1);
  ASSERT(*fget == ftest, "cbuffer_pushagain_1");
  ASSERT(cbuffer_tail(&cb) == 0, "cbuffer_pushagain_tail");

  ftest = 0;
  ASSERT(!cbuffer_pop(&cb, &ftest), "cbuffer_pop");
  ASSERT(cb.length == 1, "cbuffer_pop_length");
  ASSERT(cb.head == 2, "cbuffer_pop_head");
  ASSERT(ftest == 1.5, "cbuffer_pop_value: %f vs %f", ftest, 1.5);
  ASSERT(cbuffer_tail(&cb) == 1, "cbuffer_pop_tail");
  ASSERT(cb.element_size == sizeof(float), "cbuffer_pop_element_size_sanity");

  ftest = 0;
  ASSERT(!cbuffer_pop(&cb, &ftest), "cbuffer_pop2");
  ASSERT(cb.length == 0, "cbuffer_pop2_length");
  ASSERT(cb.head == 2, "cbuffer_pop2_head");
  ASSERT(ftest == 84930, "cbuffer_pop2_value: %f vs %f", ftest, 84930.0f);
  ASSERT(cbuffer_tail(&cb) == 2, "cbuffer_pop2_tail");

  ASSERT(cbuffer_pop(&cb, &ftest), "cbuffer_pop3");

  cbuffer_free(&cb);
  ASSERT(cb.queue == NULL, "cbuffer_free");


  sbuffer sb;

  ASSERT(!sbuffer_init(&sb, sizeof(float), 100), "sbuffer_init");
  ASSERT(sb.element_size == 4, "sbuffer_init_element_size");
  ASSERT(sb.capacity == 100, "sbuffer_init_capacity");
  ASSERT(sb.next == 0, "sbuffer_init_head");

  ASSERT(sbuffer_get(&sb, 100) == NULL, "sbuffer_get_oob");
  ASSERT(sbuffer_get(&sb, 999) == NULL, "sbuffer_get_oob_big");
  // Now make sure EVERYTHING empty
  for(int i = 0; i < 100; i++) {
    ASSERT(!sbuffer_has(&sb, i), "sbuffer_has_%d", i);
    ASSERT(sbuffer_get(&sb, i) == NULL, "sbuffer_get_%d", i);
  }

  ftest = 1.5;
  uint32_t sindex;
  ASSERT(!sbuffer_add(&sb, (void *)&ftest, &sindex), "sbuffer_add");
  ASSERT(sindex == 0, "sbuffer_add_index");
  fget = (float *)sbuffer_get(&sb, sindex);
  ASSERT(*fget == ftest, "sbuffer_add");
  ASSERT(sbuffer_has(&sb, sindex), "sbuffer_add_has");

  ftest = 84930;
  ASSERT(!sbuffer_add(&sb, (void *)&ftest, &sindex), "sbuffer_add2");
  ASSERT(sindex == 1, "sbuffer_add2_index");
  fget = (float *)sbuffer_get(&sb, sindex);
  ASSERT(*fget == ftest, "sbuffer_add2");
  ASSERT(sbuffer_has(&sb, sindex), "sbuffer_add2_has");

  ASSERT(!sbuffer_remove(&sb, 0), "sbuffer_remove0");
  ASSERT(sbuffer_remove(&sb, 0), "sbuffer_remove0_fail");
  ASSERT(!sbuffer_has(&sb, 0), "sbuffer_remove0_has");
  ASSERT(!sbuffer_remove(&sb, 1), "sbuffer_remove1");
  ASSERT(sbuffer_remove(&sb, 1), "sbuffer_remove1_fail");
  ASSERT(!sbuffer_has(&sb, 1), "sbuffer_remove1_has");

  for(int i = 0; i < 100; i++) {
    ftest = i;
    ASSERT(!sbuffer_add(&sb, (void *)&ftest, &sindex), "sbuffer_addall_%d", i);
    ASSERT(sbuffer_has(&sb, sindex), "sbuffer_addall_%d_has", i);
    ASSERT(*(float *)sbuffer_get(&sb, sindex) == ftest, "sbuffer_addall_%d_get", i);
  }

  // Remove two random elements, should be able to insert two now
  ASSERT(!sbuffer_remove(&sb, 55), "sbuffer_remove55");
  ASSERT(!sbuffer_remove(&sb, 77), "sbuffer_remove77");
  ftest = 1000;
  ASSERT(!sbuffer_add(&sb, (void *)&ftest, &sindex), "sbuffer_addhole55");
  ASSERT(sindex == 55, "sbuffer_addhole55_sindex");
  ASSERT(!sbuffer_add(&sb, (void *)&ftest, &sindex), "sbuffer_addhole77");
  ASSERT(sindex == 77, "sbuffer_addhole77_sindex");
  ASSERT(sbuffer_add(&sb, (void *)&ftest, &sindex), "sbuffer_addhole_fail");

  sbuffer_free(&sb);
  ASSERT(sb.buffer == NULL, "sbuffer_free_buffer");
  ASSERT(sb.fill == NULL, "sbuffer_free_fill");

  sortstruct sorting[10];
  for(int i = 0; i < 10; i++) {
    sorting[i].order = i;
    sorting[i].value = 10.5 - i;
  }

  // Ensure the sort is stable and doesn't break already-sorted lists
  // (nothing should move)
  LIST_SORT(sorting, 10, sortstruct, SORTSTRUCT_COMPARE);
  for(int i = 0; i < 10; i++) {
    ASSERT(sorting[i].order == i, "LIST_SORT(ordered %d)", i);
    ASSERT(sorting[i].value == 10.5 - i, "LIST_SORT(ordered-value %d)", i);
  }

  // Now put a weird value at the front, it should sift to the end.
  sorting[0].order = 99;
  LIST_SORT(sorting, 10, sortstruct, SORTSTRUCT_COMPARE);
  for(int i = 0; i < 9; i++) {
    ASSERT(sorting[i].order == i + 1, "LIST_SORT(oneset %d)", i);
    ASSERT(sorting[i].value == 10.5 - (i + 1), "LIST_SORT(oneset-value %d)", i);
  }
  ASSERT(sorting[9].order == 99, "LIST_SORT(oneset %d)", 99);
  ASSERT(sorting[9].value == 10.5 - 0, "LIST_SORT(oneset-value %d)", 99);

  // Now reset array but in reverse
  for(int i = 0; i < 10; i++) {
    sorting[i].order = 9 - i;
    sorting[i].value = 10.5 - i;
  }

  LIST_SORT(sorting, 10, sortstruct, SORTSTRUCT_COMPARE);
  for(int i = 0; i < 10; i++) {
    ASSERT(sorting[i].order == i, "LIST_SORT(reversed %d)", i);
    ASSERT(sorting[i].value == 1.5 + i, "LIST_SORT(reversed-value %d)", i);
  }

  // Now pure random? Or something...
  sorting[0].order = 43;
  sorting[1].order = 9;
  sorting[2].order = 35;
  sorting[3].order = 17;
  sorting[4].order = 55;
  sorting[5].order = 0;
  sorting[6].order = 11;
  sorting[7].order = 7;
  sorting[8].order = 99;
  sorting[9].order = 77;

  sorting[5].value = 0;
  sorting[7].value = 1;
  sorting[1].value = 2;
  sorting[6].value = 3;
  sorting[3].value = 4;
  sorting[2].value = 5;
  sorting[0].value = 6;
  sorting[4].value = 7;
  sorting[9].value = 8;
  sorting[8].value = 9;

  LIST_SORT(sorting, 10, sortstruct, SORTSTRUCT_COMPARE);
  for(int i = 0; i < 10; i++) {
    //ASSERT(sorting[i].order == i, "LIST_SORT(reversed %d)", i);
    ASSERT(sorting[i].value == i, "LIST_SORT(random-value %d)", i);
  }
}
