// haloopdy 2024

#ifndef HALOO3D_H
#define HALOO3D_H

// We want to use the unions for simplicity
#define MATHC_USE_UNIONS

#include "mathc.h"
#include <float.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define IS2POW(x) (!(x & (x - 1)) && x)
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

#define eprintf(...) fprintf(stderr, __VA_ARGS__);

// ----------------------
//   Vecs and such
// ----------------------

// These aren't necessarily hard limits; that's 65536
#define H3D_OBJ_MAXVERTICES 8192
#define H3D_OBJ_MAXFACES 8192

typedef mfloat_t hvec4f[4];
typedef mfloat_t hvec3f[3];

// A full vertex with all information inside
typedef struct {
  struct vec4 pos;
  struct vec3 tex;
} haloo3d_vertexf;

// A vertex which is made up of indexes into the obj
typedef struct {
  uint16_t posi;
  uint16_t texi;
  uint16_t normi;
} haloo3d_vertexi;

// A face which is made up of indexes into the obj
typedef haloo3d_vertexi haloo3d_facei[3];
typedef haloo3d_vertexf haloo3d_facef[3];

// An object definition, where every face is a simple
// index into the internal structures
typedef struct {
  uint16_t numvertices;
  uint16_t numvtextures;
  uint16_t numfaces;
  uint16_t numvnormals;
  struct vec4 *vertices;
  struct vec3 *vtexture;
  // vec3f *vnormal;
  haloo3d_facei *faces;
} haloo3d_obj;

// Generate a "static" face independent of the obj, useful for functions which
// work on faces rather than triangles
static inline void haloo3d_obj_facef(haloo3d_obj *obj, haloo3d_facei face,
                                     haloo3d_facef out) {
  out[0].pos = obj->vertices[face[0].posi];
  out[1].pos = obj->vertices[face[1].posi];
  out[2].pos = obj->vertices[face[2].posi];
  out[0].tex = obj->vtexture[face[0].texi];
  out[1].tex = obj->vtexture[face[1].texi];
  out[2].tex = obj->vtexture[face[2].texi];
}

// If you're not using homogenous coordinates, this will fix depth issues
// in the triangle renderer, which expects w to be some value relating to
// perspective. TODO: eventually just fix z so it works!
static inline void haloo3d_facef_fixw(haloo3d_facef face) {
  face[0].pos.w = face[0].pos.z + 1;
  face[1].pos.w = face[1].pos.z + 1;
  face[2].pos.w = face[2].pos.z + 1;
}

// ----------------------
//   Colors
// ----------------------

#define H3DC_R(c) ((((c) >> 4) & 0xF0) | 0x07)
#define H3DC_G(c) (((c) & 0xF0) | 0x07)
#define H3DC_B(c) ((((c) << 4) & 0xF0) | 0x07)
#define H3DC_RGB(r, g, b)                                                      \
  (0xF000 | (((r) & 0xF) << 8) | (((g) & 0xF) << 4) | ((b) & 0xF))

// "scale" a color by a given intensity. it WILL clip...
static inline uint16_t haloo3d_col_scale(uint16_t col, mfloat_t scale) {
  uint16_t r = ((col >> 8) & 0xf) * scale;
  uint16_t g = ((col >> 4) & 0xf) * scale;
  uint16_t b = (col & 0xf) * scale;
  return H3DC_RGB(r, g, b);
}

// ----------------------
//   Math
// ----------------------

static inline void haloo3d_viewport_into(mfloat_t *v, int width, int height) {
  //(v *Vec3f) ViewportSelf(width, height int) {
  v[0] = (v[0] + 1.0) / 2.0 * width;
  v[1] = (1.0 - ((v[1] + 1.0) / 2.0)) * height;
  // v.X = (v.X + 1) / 2 * float32(width)
  // v.Y = (1 - (v.Y+1)/2) * float32(height)
  //  Don't touch Z or whatever
}

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
static inline uint16_t haloo3d_fb_get(haloo3d_fb *fb, int x, int y) {
  return fb->buffer[x + y * fb->width];
}

static inline uint16_t haloo3d_wb_get(haloo3d_fb *fb, int x, int y) {
  return fb->wbuffer[x + y * fb->width];
}

// Set a value in the framebuffer at the given index
static inline void haloo3d_fb_set(haloo3d_fb *fb, int x, int y, uint16_t v) {
  fb->buffer[x + y * fb->width] = v;
}

static inline void haloo3d_wb_set(haloo3d_fb *fb, int x, int y, mfloat_t v) {
  fb->wbuffer[x + y * fb->width] = v;
}

// Get a value based on uv coordinates. Does not perform any smoothing
static inline uint16_t haloo3d_fb_getuv(haloo3d_fb *fb, mfloat_t u,
                                        mfloat_t v) {
  uint16_t x = (uint16_t)(fb->width * u) & (fb->width - 1);
  uint16_t y = (uint16_t)(fb->height * (1 - v)) & (fb->height - 1);
  // eprintf("%d %d | %f %f\n", x, y, u, v);
  return fb->buffer[x + y * fb->width];
}

// Get the total size in elements of any buffer inside (framebuffer or
// otherwise)
static inline int haloo3d_fb_size(haloo3d_fb *fb) {
  return fb->width * fb->height;
}

// Initialize a framebuffer with a symmetric data buffer and depth buffer
void haloo3d_fb_init(haloo3d_fb *fb, uint16_t width, uint16_t height);
// Free all the buffers created etc
void haloo3d_fb_free(haloo3d_fb *fb);
// Initialize a framebuffer for use as a texture. This makes the zbuffer null,
// but you can otherwise use it as normal
void haloo3d_fb_init_tex(haloo3d_fb *fb, uint16_t width, uint16_t height);

// Clear the wbuffer
static inline void haloo3d_fb_cleardepth(haloo3d_fb *fb) {
  // Apparently memset isn't allowed, and the compiler will optimize this
  // for us?
  const size_t len = haloo3d_fb_size(fb);
  float *const db = fb->wbuffer;
  for (size_t i = 0; i < len; i++) {
    db[i] = 65536; // Actual value doesn't matter, so long as it's large
  }
}

// ----------------------
//  Rendering
// ----------------------

// Top left corner of bounding box, but only x and y are computed
static inline struct vec2 haloo3d_boundingbox_tl(mfloat_t *v0, mfloat_t *v1,
                                                 mfloat_t *v2) {
  return (struct vec2){.x = MIN(MIN(v0[0], v1[0]), v2[0]),
                       .y = MIN(MIN(v0[1], v1[1]), v2[1])};
}

// Bottom right corner of bounding box, but only x and y are computed
static inline struct vec2 haloo3d_boundingbox_br(mfloat_t *v0, mfloat_t *v1,
                                                 mfloat_t *v2) {
  return (struct vec2){.x = MAX(MAX(v0[0], v1[0]), v2[0]),
                       .y = MAX(MAX(v0[1], v1[1]), v2[1])};
}

// Edge function for a line between points v0 and v1. Positive if on the
// "right" side (counter-clockwise winding)
static inline mfloat_t haloo3d_edgefunc(mfloat_t *v0, mfloat_t *v1,
                                        mfloat_t *p) {
  return (p[0] - v0[0]) * (v1[1] - v0[1]) - (p[1] - v0[1]) * (v1[0] - v0[0]);
}

// Calculate the increment amount in x and y direction for line between two
// given points
static inline struct vec2 haloo3d_edgeinc(mfloat_t *v0, mfloat_t *v1) {
  return (struct vec2){.x = (v1[1] - v0[1]), .y = -(v1[0] - v0[0])};
}

// Edge function for a line between points v0 and v1. Positive if on the
// "right" side (counter-clockwise winding)
static inline mint_t haloo3d_edgefunci(mint_t *v0, mint_t *v1, mint_t *p) {
  return (p[0] - v0[0]) * (v1[1] - v0[1]) - (p[1] - v0[1]) * (v1[0] - v0[0]);
}

// Calculate the increment amount in x and y direction for line between two
// given points
static inline struct vec2i haloo3d_edgeinci(mint_t *v0, mint_t *v1) {
  return (struct vec2i){.x = (v1[1] - v0[1]), .y = -(v1[0] - v0[0])};
}

// Draw a textured triangle into the given framebuffer using the given face
void haloo3d_texturedtriangle(haloo3d_fb *fb, haloo3d_fb *texture,
                              mfloat_t intensity, haloo3d_facef face);

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
