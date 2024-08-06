// haloopdy 2024

#ifndef HALOO3D_H
#define HALOO3D_H

#include <float.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// ----------------------
//   Colors
// ----------------------

#define H3DC_R(c) ((((c) >> 4) & 0xF0) | 0x07)
#define H3DC_G(c) (((c) & 0xF0) | 0x07)
#define H3DC_B(c) ((((c) << 4) & 0xF0) | 0x07)

// ----------------------
//  Framebuffer
// ----------------------

// The framebuffer object, which stores stuff like the 16 bit
// framebuffer, the depth buffer, etc
typedef struct {
  uint16_t *buffer; // actual buffer (managed manually)
  uint16_t width;   // width of the framebuffer
  uint16_t height;  // height of the framebuffer
  float *wbuffer;   // Depth buffer, using w value instead of z
} haloo3d_fb;

// Get a value from the framebuffer at the given index
inline uint16_t haloo3d_fb_get(haloo3d_fb *fb, int x, int y) {
  return fb->buffer[x + y * fb->width];
}

// Set a value in the framebuffer at the given index
inline void haloo3d_fb_set(haloo3d_fb *fb, int x, int y, uint16_t v) {
  fb->buffer[x + y * fb->width] = v;
}

// Get the total size in elements of any buffer inside (framebuffer or
// otherwise)
inline int haloo3d_fb_size(haloo3d_fb *fb) { return fb->width * fb->height; }

// Initialize a framebuffer with a symmetric data buffer and depth buffer
void haloo3d_fb_init(haloo3d_fb *fb, uint16_t width, uint16_t height);
// Free all the buffers created etc
void haloo3d_fb_free(haloo3d_fb *fb);

// Clear the wbuffer
inline void haloo3d_fb_cleardepth(haloo3d_fb *fb) {
  // Apparently memset isn't allowed, and the compiler will optimize this
  // for us?
  const size_t len = sizeof(float) * haloo3d_fb_size(fb);
  float *const db = fb->wbuffer;
  for (size_t i = 0; i < len; i++) {
    db[i] = FLT_MAX;
  }
}

// ----------------------
// Some helper functions
// ----------------------

// Die with an error (most calls in library will die on fatal error)
#define dieerr(...)                                                            \
  {                                                                            \
    fprintf(stderr, __VA_ARGS__);                                              \
    exit(1);                                                                   \
  }
#define mallocordie(ass, size)                                                 \
  {                                                                            \
    ass = malloc(size);                                                        \
    if (ass == NULL) {                                                         \
      dieerr("Could not allocate mem, size %ld", size);                        \
    }                                                                          \
  }

#endif
