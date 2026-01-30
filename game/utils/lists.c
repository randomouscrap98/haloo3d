#include "lists.h"
#include "log.h"
#include "utils.h"

// --------------------------------------
//            CBUFFER
// --------------------------------------

int cbuffer_init(cbuffer * q, uint16_t element_size, uint32_t capacity) {
  q->queue = NULL;
  q->element_size = element_size;
  q->capacity = capacity;
  q->queue = (uint8_t *)malloc(element_size * capacity);
  if(q->queue == NULL) { 
    cbuffer_free(q); 
    return 1; 
  }
  cbuffer_clear(q);
  return 0;
}

uint32_t cbuffer_tail(cbuffer * q) {
  return (q->head - q->length + q->capacity) % q->capacity;
}

void * cbuffer_get(cbuffer * q, uint32_t index) {
  // Don't bother with weird situation
  if(q->length == 0 || index >= q->capacity) {
    return NULL;
  }
  uint32_t tail = cbuffer_tail(q);
  uint32_t head = q->head;
  if(head < tail) {
    // Push both outside the range of capacity so math works
    head += q->capacity;
    index += q->capacity;
  }
  if(index < tail || index >= head) {
    return NULL;
  }
  return (void *)(q->queue + ((index % q->capacity) * q->element_size));
}

int cbuffer_push(cbuffer * q, void * item) {
  if(q->length >= q->capacity) {
    return 1;
  }
  memcpy(q->queue + (q->head * q->element_size), item, q->element_size);
  q->head = (q->head + 1) % q->capacity;
  q->length++;
  return 0;
}

int cbuffer_pop(cbuffer * q, void * item) {
  if(q->length == 0) {
    return 1;
  }
  void * src = cbuffer_get(q, cbuffer_tail(q));
  if(src == NULL) {
    return 1;
  }
  memcpy(item, src, q->element_size);
  q->length--;
  return 0;
}

void cbuffer_clear(cbuffer * q) {
  q->head = 0;
  q->length = 0;
}

void cbuffer_free(cbuffer * q) {
  NULLFREE(q->queue);
}

// --------------------------------------
//            SBUFFER
// --------------------------------------

int sbuffer_init(sbuffer * q, uint16_t element_size, uint32_t capacity) {
  q->buffer = NULL;
  q->fill = NULL;
  q->element_size = element_size;
  q->capacity = capacity;
  q->buffer = (uint8_t *)malloc(element_size * capacity);
  if(q->buffer == NULL) { 
    sbuffer_free(q); 
    return 1; 
  }
  uint32_t fcap = sbuffer_fillcapacity(q);
  q->fill = (uint8_t *)malloc(fcap);
  if(q->fill == NULL) { 
    sbuffer_free(q); 
    return 1; 
  }
  sbuffer_clear(q);
  return 0;
}

uint32_t sbuffer_fillcapacity(sbuffer * q) { 
  // Always 1 higher than needed but whatever
  return 1 + q->capacity / 8; 
}

int sbuffer_has(sbuffer * q, uint32_t index) {
  return index < q->capacity && q->fill[index / 8] & (1 << (index & 7));
}

void * sbuffer_get(sbuffer * q, uint32_t index) {
  if(!sbuffer_has(q, index)) {
    return NULL;
  }
  return (void *)(q->buffer + (index * q->element_size));
}

void * sbuffer_get_new(sbuffer * q, uint32_t * index) {
  int result = sbuffer_reserve(q, index);
  if(result) return NULL;
  return sbuffer_get(q, result);
}

int sbuffer_reserve(sbuffer * q, uint32_t * index_out) {
  // Find SOME place where thing isn't found.
  for(uint32_t i = 0; i < q->capacity; i++) {
    uint32_t onext = q->next;
    q->next = (q->next + 1) % q->capacity;
    if(!sbuffer_has(q, onext)) {
      q->fill[onext / 8] |= 1 << (onext & 7);
      *index_out = onext;
      return 0;
    }
  }
  // If you searched the entire thing and didn't find a spot,
  // then the buffer is full.
  return 1;
}

int sbuffer_add(sbuffer * q, void * item, uint32_t * index_out) {
  int result = sbuffer_reserve(q, index_out);
  if(result) return result;
  memcpy(q->buffer + ((*index_out) * q->element_size), item, q->element_size);
  return 0;
}

int sbuffer_remove(sbuffer * q, uint32_t index) {
  if(!sbuffer_has(q, index)) {
    return 1;
  }
  q->fill[index / 8] &= ~(1 << (index & 7));
  return 0;
}

void sbuffer_clear(sbuffer * q) {
  memset(q->fill, 0, sbuffer_fillcapacity(q));
  q->next = 0;
}

void sbuffer_free(sbuffer * q) {
  NULLFREE(q->buffer);
  NULLFREE(q->fill);
}

// --------------------------------------
//            VBUFFER
// --------------------------------------

int vbuffer_init(vbuffer * v, uint16_t element_size) {
  v->buffer = NULL;
  v->element_size = element_size;
  v->capacity = VBUF_INIT_CAPACITY;
  v->buffer = (uint8_t *)malloc(element_size * v->capacity);
  if(v->buffer == NULL) { 
    vbuffer_free(v); 
    return 1; 
  }
  vbuffer_clear(v);
  return 0;
}

void vbuffer_clear(vbuffer * v) {
  v->length = 0;
}

// Instead of copying a value in to add, simply reserve the spot
// and return a pointer back. Saves some useless data copy if you're
// not going to use it
int vbuffer_reserve(vbuffer * v, void ** out) {
  if(v->length >= v->capacity) {
    v->capacity += (int)(v->capacity * VBUF_GROW_FACTOR);
    v->buffer = (uint8_t *)realloc(v->buffer, v->capacity * v->element_size);
    if (v->buffer == NULL) {
      logerror("Couldn't reallocate vbuffer during reserve (out of memory?)");
      return 1;
    }
  }
  *out = (void*)(v->buffer + (v->length * v->element_size));
  v->length++;
  return 0;
}

int vbuffer_push(vbuffer * v, void * item) {
  void * dest;
  int result = vbuffer_reserve(v, &dest);
  if(result) {
    return result;
  }
  memcpy(dest, item, v->element_size);
  return 0;
}

int vbuffer_decrement(vbuffer * v) {
  if(v->length == 0) {
    return 1;
  }
  v->length--;
  return 0;
}

int vbuffer_pop(vbuffer * v, void * item) {
  if(v->length == 0) {
    return 1;
  }
  v->length--;
  memcpy(item, v->buffer + (v->length * v->element_size), v->element_size);
  return 0;
}

int vbuffer_bag_remove(vbuffer * v, uint32_t index) {
  if(index >= v->length) {
    return 1;
  }
  if(index == v->length -1) {
    return vbuffer_decrement(v);
  }
  v->length--;
  memcpy(v->buffer + (index * v->element_size), v->buffer + (v->length * v->element_size), v->element_size);
  return 0;
}

void vbuffer_free(vbuffer * v) {
  NULLFREE(v->buffer);
}
