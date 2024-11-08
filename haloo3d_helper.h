#ifndef HALOO3D_HELPER_H
#define HALOO3D_HELPER_H

// Helper macros for various aspects of haloo3d. Since it is JUST macros,
// you can safely include it for any combination of haloo3d sublibraries

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define IS2POW(x) (!(x & (x - 1)) && x)
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define CLAMP(v, min, max) (((v) < min) ? min : ((v) > max) ? max : (v))
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

// Initialize an FB with the given width, height, and pixel byte size
#define H3D_FB_INIT(fb, w, h, ps)                                              \
  {                                                                            \
    fb->width = width;                                                         \
    fb->height = height;                                                       \
    mallocordie(fb->buffer, (ps) * H3D_FB_SIZE(fb));                           \
    mallocordie(fb->dbuffer, sizeof(float_t) * H3D_FB_SIZE(fb));               \
  }

#define H3D_FB_FREE(fb)                                                        \
  {                                                                            \
    free(fb->buffer);                                                          \
    if (fb->dbuffer != NULL) {                                                 \
      free(fb->dbuffer);                                                       \
    }                                                                          \
  }

// Initialize fb to hold a texture, meaning it has no depth buffer and the
// width/height must be a power of 2
#define H3D_FB_TEXINIT(fb, w, h)                                               \
  {                                                                            \
    if (!IS2POW(width) || !IS2POW(height)) {                                   \
      dieerr("Texture width and height must be power of 2: %dX%d\n", width,    \
             height);                                                          \
    }                                                                          \
    fb->width = width;                                                         \
    fb->height = height;                                                       \
    fb->dbuffer = NULL;                                                        \
    mallocordie(fb->buffer, sizeof(uint16_t) * H3D_FB_SIZE(fb));               \
  }

// Convert given color to full transparency in whole fb
#define H3D_FB_TOTRANSPARENT(fb, col)                                          \
  {                                                                            \
    const int size = h3d_fb_size(fb);                                          \
    for (int _i = 0; _i < size; _i++) {                                        \
      if (fb->buffer[_i] == col)                                               \
        fb->buffer[_i] = 0;                                                    \
    }                                                                          \
  }

// ========================================
// |            OBJECT (MODEL)            |
// ========================================

#define H3D_OBJ_ADDVERT(obj, v)                                                \
  memcpy(obj->vertices[obj->numvertices++], v, sizeof(vec4))

#define H3D_OBJ_ADDVTEX(obj, t)                                                \
  memcpy(obj->vtexture[obj->numvtextures++], t, sizeof(vec3))

#define H3D_OBJ_ADDFACE(obj, f)                                                \
  memcpy(obj->faces[obj->numfaces++], face, sizeof(h3d_objface))

// Initialize object to have all 0 counts but pre-allocate the given number
// of faces and vertices (vert count used for tex and normal)
#define H3D_OBJ_INIT(obj, numf, numv)                                          \
  {                                                                            \
    obj->numfaces = 0;                                                         \
    obj->numvertices = 0;                                                      \
    obj->numvtextures = 0;                                                     \
    obj->numvnormals = 0;                                                      \
    mallocordie(obj->faces, sizeof(h3d_objface) * numf);                       \
    mallocordie(obj->vertices, sizeof(vec4) * numv);                           \
    mallocordie(obj->vtexture, sizeof(vec3) * numv);                           \
    mallocordie(obj->vnormals, sizeof(vec3) * numv);                           \
  }

#define H3D_OBJ_INITMAX(obj)                                                   \
  H3D_OBJ_INIT(obj, H3D_OBJ_MAXFACES, H3D_OBJ_MAXVERTICES)

#define H3D_OBJ_FREE(obj)                                                      \
  {                                                                            \
    free(obj->faces);                                                          \
    free(obj->vertices);                                                       \
    free(obj->vtexture);                                                       \
    free(obj->vnormals);                                                       \
  }

// Resize all arrays so they're exactly the size needed for the numbers
// in the given obj
#define H3D_OBJ_SHRINK(obj)                                                    \
  {                                                                            \
    reallocordie(obj->faces, sizeof(h3d_objface) * MAX(1, obj->numfaces));     \
    reallocordie(obj->vertices, sizeof(vec4) * MAX(1, obj->numvertices));      \
    reallocordie(obj->vtexture, sizeof(vec3) * MAX(1, obj->numvtextures));     \
    reallocordie(obj->vnormals, sizeof(vec3) * MAX(1, obj->numvnormals));      \
  }

// Load a whole object file into an unitialized object using the given file
// pointer. Assumes the file pointer is at the point you want it at
#define H3D_OBJ_LOAD(obj, f)                                                   \
  {                                                                            \
    H3D_OBJ_INITMAX(obj);                                                      \
    char line[H3D_OBJ_MAXLINESIZE];                                            \
    char err[1024];                                                            \
    while (fgets(line, H3D_OBJ_MAXLINESIZE, f)) {                              \
      h3d_obj_parseline(obj, line, err, 1024);                                 \
    }                                                                          \
    H3D_OBJ_SHRINK(obj);                                                       \
  }

// Load a whole object file into an unitialized object using the given string
#define H3D_OBJ_LOADSTRING(obj, s)                                             \
  {                                                                            \
    H3D_OBJ_INITMAX(obj);                                                      \
    char line[H3D_OBJ_MAXLINESIZE];                                            \
    char err[1024];                                                            \
    char *endline;                                                             \
    int running = 1;                                                           \
    while (running) {                                                          \
      endline = strchr(str, '\n');                                             \
      if (endline) {                                                           \
        strncpy(line, str, MIN(H3D_OBJ_MAXLINESIZE - 1, endline - str));       \
        str = endline + 1;                                                     \
      } else {                                                                 \
        strncpy(line, str, MIN(H3D_OBJ_MAXLINESIZE - 1, strlen(str)));         \
        running = 0;                                                           \
      }                                                                        \
      h3d_obj_parseline(obj, line, err, 1024);                                 \
    }                                                                          \
    H3D_OBJ_SHRINK(obj);                                                       \
    eprintf("Read from object string: v=%d, f=%d, t=%d\n", obj->numvertices,   \
            obj->numfaces, obj->numvtextures);                                 \
  }

// Open a simple file and read the wavefront obj into an unitialized obj
#define H3D_OBJ_LOADFILE(obj, filename)                                        \
  {                                                                            \
    FILE *f = fopen(filename, "r");                                            \
    if (f == NULL) {                                                           \
      dieerr("Can't open %s for reading object\n", filename);                  \
    }                                                                          \
    H3D_OBJ_LOAD(obj, f);                                                      \
    fclose(f);                                                                 \
    eprintf("Read from object file %s\n", filename);                           \
  }

#endif
