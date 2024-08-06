// haloopdy 2024

#ifndef HALOO3D_H
#define HALOO3D_H

#include "mathc.h"
#include <float.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// ----------------------
//   Vecs and such
// ----------------------

// These aren't necessarily hard limits; that's 65536
#define H3D_OBJ_MAXVERTICES 8192
#define H3D_OBJ_MAXFACES 8192

typedef mfloat_t vec4f[4];
typedef mfloat_t vec3f[3];

// A full vertex with all information inside
typedef struct {
  vec4f pos;
  vec3f tex;
} haloo3d_vertexf;

// A vertex which is made up of indexes into the obj
typedef struct {
  uint16_t posi;
  uint16_t texi;
  uint16_t normi;
} haloo3d_vertexi;

// A face which is made up of indexes into the obj
typedef haloo3d_vertexi haloo3d_facei[3];

// An object definition, where every face is a simple
// index into the internal structures
typedef struct {
  uint16_t numvertices;
  uint16_t numvtextures;
  uint16_t numfaces;
  uint16_t numvnormals;
  vec4f *vertices;
  vec3f *vtexture;
  // vec3f *vnormal;
  haloo3d_facei *faces;
} haloo3d_obj;

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
  uint16_t *buffer;  // actual buffer (managed manually)
  uint16_t width;    // width of the framebuffer
  uint16_t height;   // height of the framebuffer
  mfloat_t *wbuffer; // Depth buffer, using w value instead of z
} haloo3d_fb;

// Get a value from the framebuffer at the given index
inline uint16_t haloo3d_fb_get(haloo3d_fb *fb, int x, int y) {
  return fb->buffer[x + y * fb->width];
}

// Set a value in the framebuffer at the given index
inline void haloo3d_fb_set(haloo3d_fb *fb, int x, int y, uint16_t v) {
  fb->buffer[x + y * fb->width] = v;
}

// Get a value based on uv coordinates. Does not perform any smoothing
inline uint16_t haloo3d_fb_getuv(haloo3d_fb *fb, mfloat_t u, mfloat_t v) {
  uint16_t x = (uint16_t)(fb->width * u) & (fb->width - 1);
  uint16_t y = (uint16_t)(fb->height * (1 - v)) & (fb->height - 1);
  return fb->buffer[x + y * fb->width];
}

// Get the total size in elements of any buffer inside (framebuffer or
// otherwise)
inline int haloo3d_fb_size(haloo3d_fb *fb) { return fb->width * fb->height; }

// Initialize a framebuffer with a symmetric data buffer and depth buffer
void haloo3d_fb_init(haloo3d_fb *fb, uint16_t width, uint16_t height);
// Free all the buffers created etc
void haloo3d_fb_free(haloo3d_fb *fb);
// Initialize a framebuffer for use as a texture. This makes the zbuffer null,
// but you can otherwise use it as normal
void haloo3d_fb_init_tex(haloo3d_fb *fb, uint16_t width, uint16_t height);

// Clear the wbuffer
inline void haloo3d_fb_cleardepth(haloo3d_fb *fb) {
  // Apparently memset isn't allowed, and the compiler will optimize this
  // for us?
  const size_t len = sizeof(float) * haloo3d_fb_size(fb);
  float *const db = fb->wbuffer;
  for (size_t i = 0; i < len; i++) {
    db[i] = FLT_MAX; // Actual value doesn't matter, so long as it's large
  }
}

// ----------------------
// Some helper functions
// ----------------------

#define IS2POW(x) (!(x & (x - 1)) && x)
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

#define eprintf(...) fprintf(stderr, __VA_ARGS__);

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
      dieerr("Could not allocate mem, size %ld\n", size);                      \
    }                                                                          \
  }
#define reallocordie(ass, size)                                                \
  {                                                                            \
    ass = realloc(ass, size);                                                  \
    if (ass == NULL) {                                                         \
      dieerr("Could not reallocate mem, size %ld\n", size);                    \
    }                                                                          \
  }

#endif
