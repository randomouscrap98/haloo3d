#ifndef HALOO3D_HELPER_H
#define HALOO3D_HELPER_H

// Extensions with some opinioned ways of handling things. use these if
// you want some shortcuts, but know that they assume you want things to
// work in a specific way. This is dependent on all features of
// baseline haloo3d

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "haloo3d.h"
#include "haloo3d_3d.h"
#include "haloo3d_obj.h"

#define eprintf(...) fprintf(stderr, __VA_ARGS__);

// Die with an error (most calls in library will die on fatal error)
#define dieerr(...)                                                            \
  {                                                                            \
    fprintf(stderr, __VA_ARGS__);                                              \
    exit(1);                                                                   \
  }
// Attempt to malloc the given assignment or die trying
#define mallocordie(ass, size)                                                 \
  {                                                                            \
    ass = malloc(size);                                                        \
    if (ass == NULL) {                                                         \
      dieerr("Could not allocate mem, size %d\n", (int)(size));                \
    }                                                                          \
  }
// Attempt to realloc the given assignment or die trying
#define reallocordie(ass, size)                                                \
  {                                                                            \
    ass = realloc(ass, size);                                                  \
    if (ass == NULL) {                                                         \
      dieerr("Could not reallocate mem, size %d\n", (int)(size));              \
    }                                                                          \
  }

// Print vector (must be array)
#define printvec4(v) eprintf("(%f, %f, %f, %f)", v[0], v[1], v[2], v[3])

// Print matrix (must be array)
#define printmatrix(m)                                                         \
  {                                                                            \
    eprintf("%f %f %f %f\n", m[0], m[1], m[2], m[3]);                          \
    eprintf("%f %f %f %f\n", m[4], m[5], m[6], m[7]);                          \
    eprintf("%f %f %f %f\n", m[8], m[9], m[10], m[11]);                        \
    eprintf("%f %f %f %f\n", m[12], m[13], m[14], m[15]);                      \
  }

// clang-format off
#define H3D_DITHER4x4 { \
    0x00, 0x00, 0x00, 0x00, \
    0x88, 0x00, 0x00, 0x00, \
    0x88, 0x00, 0x22, 0x00, \
    0xAA, 0x00, 0x22, 0x00, \
    0xAA, 0x00, 0xAA, 0x00, \
    0xAA, 0x44, 0xAA, 0x00, \
    0xAA, 0x44, 0xAA, 0x11, \
    0xAA, 0x55, 0xAA, 0x11, \
    0xAA, 0x55, 0xAA, 0x55, \
    0xEE, 0x55, 0xAA, 0x55, \
    0xEE, 0x55, 0xBB, 0x55, \
    0xFF, 0x55, 0xBB, 0x55, \
    0xFF, 0x55, 0xFF, 0x55, \
    0xFF, 0xDD, 0xFF, 0x55, \
    0xFF, 0xDD, 0xFF, 0x77, \
    0xFF, 0xFF, 0xFF, 0x77, \
    0xFF, 0xFF, 0xFF, 0xFF, \
}
// clang-format on

// ===========================================
// |                 IMAGE                   |
// ===========================================

typedef void (*h3d_fb_imgout)(uint16_t, uint8_t[4]);
typedef uint16_t (*h3d_fb_imgin)(float[4]);

// Write 16 bit color in A4R4G4B4 out to given buffer in order ARGB
static inline void h3d_fb_out_A4R4G4B4(uint16_t color, uint8_t buf[4]) {
  buf[0] = H3DC_A4(color);
  buf[1] = H3DC_R4(color);
  buf[2] = H3DC_G4(color);
  buf[3] = H3DC_B4(color);
};
// Write 16 bit color in A1R5G5B5 out to given buffer in order ARGB
static inline void h3d_fb_out_A1R5G5B5(uint16_t color, uint8_t buf[4]) {
  buf[0] = H3DC_A1(color);
  buf[1] = H3DC_R5(color);
  buf[2] = H3DC_G5(color);
  buf[3] = H3DC_B5(color);
};

// Create 16 bit color in A4R4G4B4 from given color buffer (0 to 1)
static inline uint16_t h3d_fb_in_A4R4G4B4(float buf[4]) {
  return H3DC_A4R4G4B4_F(buf[0], buf[1], buf[2], buf[3]);
};
static inline uint16_t h3d_fb_in_A1R5G5B5(float buf[4]) {
  return H3DC_A1R5G5B5_F(buf[0], buf[1], buf[2], buf[3]);
};

// Writes a P6 binary ppm from the 16 bit framebuffer. Must give the
// conversion function for color to buffer
void h3d_fb_writeppm(h3d_fb *fb, FILE *f, h3d_fb_imgout cc);
// Write a P6 binary ppm to a file. Kills whole program if it can't
void h3d_fb_writeppmfile(h3d_fb *fb, char *filename, h3d_fb_imgout cc);
// Loads a P6 binary ppm into a framebuffer
void h3d_fb_loadppm(FILE *f, h3d_fb *fb, h3d_fb_imgin cc);
// Load a P6 binary ppm into the given texture. Kills whole program
// if it can't.
void h3d_fb_loadppmfile(h3d_fb *tex, char *filename, h3d_fb_imgin cc);

// ========================================
// |            FRAMEBUFFER               |
// ========================================

// These have to be macros so they can accept different kinds of fb

// Initialize an FB with the given width, height, and pixel byte size
#define H3D_FB_INIT(fb, w, h, ps)                                              \
  {                                                                            \
    (fb)->width = w;                                                           \
    (fb)->height = h;                                                          \
    mallocordie((fb)->buffer, (ps) * H3D_FB_SIZE(fb));                         \
    mallocordie((fb)->dbuffer, sizeof(hfloat_t) * H3D_FB_SIZE(fb));            \
  }

#define H3D_FB_FREE(fb)                                                        \
  {                                                                            \
    free((fb)->buffer);                                                        \
    if ((fb)->dbuffer != NULL) {                                               \
      free((fb)->dbuffer);                                                     \
    }                                                                          \
  }

// Initialize fb to hold a texture, meaning it has no depth buffer and the
// width/height must be a power of 2
#define H3D_FB_TEXINIT(fb, w, h)                                               \
  {                                                                            \
    int width = (w);                                                           \
    int height = (h);                                                          \
    if (!H3D_IS2POW(width) || !H3D_IS2POW(height)) {                           \
      dieerr("Texture width and height must be power of 2: %dX%d\n", w, h);    \
    }                                                                          \
    (fb)->width = w;                                                           \
    (fb)->height = h;                                                          \
    (fb)->dbuffer = NULL;                                                      \
    mallocordie((fb)->buffer, sizeof(uint16_t) * H3D_FB_SIZE(fb));             \
  }

// Fast draw the ENTIRE src into the dst at x, y at the given integer scale. If
// drawing outside the bounds or too big, behavior is undefined (for now). Does
// NOT check alpha, simply a full overwrite
void h3d_fb_intscale(h3d_fb *src, h3d_fb *dst, int dstofsx, int dstofsy,
                     uint8_t scale);
// Simplified wrapper around h3d_fb_intscale for the common usecase of
// overwriting dst with source filled as much as possible, with optional
// centering. Useful when copying a small buffer used for 3d rendering into a
// larger screen buffer
void h3d_fb_fill(h3d_fb *src, h3d_fb *dst, uint8_t centered);

// ========================================
// |            OBJECT (MODEL)            |
// ========================================

// Initialize object to have all 0 counts but pre-allocate the given number
// of faces and vertices (vert count used for tex and normal)
void h3d_obj_init(h3d_obj *obj, uint32_t numf, uint32_t numv);
void h3d_obj_free(h3d_obj *obj);
// Resize all arrays so they're exactly the size needed for the numbers
// in the given obj
void h3d_obj_shrink(h3d_obj *obj);
// Load a whole object file into an unitialized object using the given file
// pointer. Assumes the file pointer is at the point you want it at
void h3d_obj_load(h3d_obj *obj, FILE *f, uint32_t maxf, uint32_t maxv);
// Load a whole object file into an unitialized object using the given string
void h3d_obj_loadstring(h3d_obj *obj, const char *s, uint32_t maxf, uint32_t maxv);
// Open a simple file and read the wavefront obj into an unitialized obj
void h3d_obj_loadfile(h3d_obj *obj, char *filename, uint32_t maxf, uint32_t maxv);

// Batch convert all vertices in an object into translated homogenous vertices.
// This is a very common operation done for triangle rendering
int h3d_obj_batchtranslate(h3d_obj *object, mat4 matrix, vec4 *out);
// Insert the entirety of an object into another. IT'S UP TO YOU TO KNOW
// IF THE DEST OBJECT HAS ENOUGH SPACE!
int h3d_obj_addobj(h3d_obj *dest, h3d_obj *src, vec3 pos, vec3 lookvec,
                    vec3 up, vec3 scale);

// ========================================
// |            3DFACE                    |
// ========================================

hfloat_t h3d_3dface_light(hfloat_t *light, hfloat_t minlight, h3d_3dface face);

#endif
