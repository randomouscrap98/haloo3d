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

// ========================================
// |            OBJECT (MODEL)            |
// ========================================

// Initialize object to have all 0 counts but pre-allocate the given number
// of faces and vertices (vert count used for tex and normal)
void h3d_obj_init(h3d_obj *obj, uint16_t numf, uint16_t numv);
void h3d_obj_init_max(h3d_obj *obj);
void h3d_obj_free(h3d_obj *obj);
// Resize all arrays so they're exactly the size needed for the numbers
// in the given obj
void h3d_obj_shrink(h3d_obj *obj);
// Load a whole object file into an unitialized object using the given file
// pointer. Assumes the file pointer is at the point you want it at
void h3d_obj_load(h3d_obj *obj, FILE *f);
// Load a whole object file into an unitialized object using the given string
void h3d_obj_loadstring(h3d_obj *obj, char *s);
// Open a simple file and read the wavefront obj into an unitialized obj
void h3d_obj_loadfile(h3d_obj *obj, char *filename);

// Batch convert all vertices in an object into translated homogenous vertices.
// This is a very common operation done for triangle rendering
int h3d_obj_batchtranslate(h3d_obj *object, mat4 matrix, vec4 *out);
// Insert the entirety of an object into another. IT'S UP TO YOU TO KNOW
// IF THE DEST OBJECT HAS ENOUGH SPACE!
void h3d_obj_addobj(h3d_obj *dest, h3d_obj *src, vec3 pos, vec3 lookvec,
                    vec3 up, vec3 scale);

#endif
