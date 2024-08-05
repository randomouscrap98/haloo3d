// haloopdy 2024

#ifndef HALOO3D_H
#define HALOO3D_H

#include <stdint.h>
#include <stdio.h>

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

// Initialize a framebuffer with a symmetric data buffer and depth buffer
void haloo3d_fb_init(haloo3d_fb *fb, uint16_t width, uint16_t height);
// Free all the buffers created etc
void haloo3d_fb_free(haloo3d_fb *fb);
// Clear the wbuffer
void haloo3d_fb_cleardepth(haloo3d_fb *fb);

#define H3DFB_IND(fb, x, y) ((x) + (y) * fb->width)
#define H3DFB_BSZ(fb) (sizeof(uint16_t) * fb->width * fb->height)
#define H3DFB_DBSZ(fb) (sizeof(float) * fb->width * fb->height)

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
