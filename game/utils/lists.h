#ifndef __LISTS_H__
#define __LISTS_H__

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define VBUF_INIT_CAPACITY 4
#define VBUF_GROW_FACTOR 1.5

// Simple circular buffer implementation. Pushing and popping
// require kinda bad math and manual memory manipulation so
// maybe don't use it too much... Push is at the front, pop
// is from the end. Not a stack, a queue
typedef struct {
  uint8_t * queue; 
  uint16_t element_size;
  uint32_t capacity;
  uint32_t head;    // Exclusive
  uint32_t length;
} cbuffer;

// Allocate circular buffer
int cbuffer_init(cbuffer * q, uint16_t element_size, uint32_t capacity);
// Get a pointer to a single element in the circular buffer. Index is offset from head
void * cbuffer_get(cbuffer * q, uint32_t index);
// Add an element to the "end" of the queue (newest)
int cbuffer_push(cbuffer * q, void * item);
// Remove + save the element from the "start" of the queue (oldest)
int cbuffer_pop(cbuffer * q, void * item);
// Get the location of the end of the array (inclusive).
uint32_t cbuffer_tail(cbuffer * q);
// Reset cbuffer (doesn't clear memory)
void cbuffer_clear(cbuffer * q);
void cbuffer_free(cbuffer * q);

// Simple sparse buffer. Filled slots are kept track of with minimal
// bits, and you can only add elements (anywhere) or remove SPECIFIC elements.
// A small optimization attempts to add elements one after the last insert
typedef struct {
  uint8_t * buffer; 
  uint8_t * fill;
  uint32_t capacity;
  uint32_t next;
  uint16_t element_size;
} sbuffer;

int sbuffer_init(sbuffer * q, uint16_t element_size, uint32_t capacity);
int sbuffer_add(sbuffer * q, void * item, uint32_t * index_out);
int sbuffer_reserve(sbuffer * q, uint32_t * index_out);
int sbuffer_remove(sbuffer * q, uint32_t index);
int sbuffer_has(sbuffer * q, uint32_t index);
void * sbuffer_get(sbuffer * q, uint32_t index);
void * sbuffer_get_new(sbuffer * q, uint32_t * index_out); // reserve + get
uint32_t sbuffer_fillcapacity(sbuffer * q);
void sbuffer_clear(sbuffer * q);
void sbuffer_free(sbuffer * q);

typedef struct {
  uint8_t * buffer; 
  uint16_t element_size;
  uint32_t capacity;
  uint32_t length;
} vbuffer;

int vbuffer_init(vbuffer * v, uint16_t element_size);
void vbuffer_clear(vbuffer * v);
int vbuffer_reserve(vbuffer * v, void ** out);
int vbuffer_push(vbuffer * v, void * item);
int vbuffer_pop(vbuffer * v, void * item);
// This removes an element at the given index and, without caring about 
// preserving the order of the vbuffer (treating it like a bag), fill it
// with the element from the end.
int vbuffer_bag_remove(vbuffer * v, uint32_t index);
int vbuffer_decrement(vbuffer * v); // Free the end without returning the element
void vbuffer_free(vbuffer * v);
//int vbuffer_get(vbuffer * v, void * item);

// --------------------------------------
//            LINKED LIST
// --------------------------------------

// Linked list is expected to always have a field called
// "next" which is a pointer to list type node. You must
// tell the macro what to do with individual nodes. Pass "free"
// for standard ll free with nodes with no internal allocation
#define LL_FOREACH(list, type, func) { \
  type * cur = list; \
  type * next; \
  while(cur) { \
    next = cur->next; \
    func(cur); /* May free/alter cur, must store next */ \
    cur = next; \
  } \
}

#define LL_LAST(list, type, ass) { \
  type * next = list; \
  while(next) { \
    ass = next; \
    next = cur->next; \
  }

#define LL_ADD(list, type, new) { \
  if(list == NULL) { \
    list = new(); \
  } else { \
    type * last; \
    LL_LAST(list, type, last); \
    last->next = new(); \
  } \
}

// --------------------------------------
//               SORT
// --------------------------------------
// Performs insertion sort (in-place, etc). The 'compare' function
// should return 1 if two consecutive elements are "in order"
#define LIST_SORT(list, length, type, compare) {  \
  for(int __i = 1; __i < length; __i++) { \
    for(int __j = __i; __j > 0; __j--) { \
      if(compare(list[__j - 1], list[__j])) break; \
      type __tmp = list[__j - 1]; \
      list[__j - 1] = list[__j]; \
      list[__j] = __tmp; \
    } \
  } \
}



#endif

