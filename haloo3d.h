// haloopdy 2024
// Header for the accompanying file haloo3d.c. Both files are required
// to make the core library. The "haloo3dex_*" files are extras and
// can be used to create the "full" library, but nothing in them
// is required for 3D rendering

#ifndef HALOO3D_H
#define HALOO3D_H

// Flags to control system at compile time

// Force system to check uncertain bounds and die if bad
// #define H3D_SANITY_CHECK

// Performance flags
// Completely disable "lighting" in fast triangle renderer, even when requested
// #define H3D_FAST_NO_COLSCALING
// Completely disable dithering in fast triangle renderer, even when requested.
// The triangles may still disappear in some circumstances
// #define H3D_FAST_NO_DITHERING
// Disable transparency checks in fast triangle renderer. Has no effect on
// dithering
// #define H3D_FAST_NO_TRANSPARENCY

// Configure the mathc library before importing
#define MATHC_USE_UNIONS
#define MATHC_NO_STRUCT_FUNCTIONS

#include "lib/mathc.h"
#include <float.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// The maximum amount of faces produced by clipping,
// though realistically this is never reached (also I don't
// even know if that's correct). It should(?) be that for
// each plane we clip against, we multiply by two. So
// however many planes we clip, that's the number we shift
#define H3D_FACEF_CLIPPLANES 5
#define H3D_FACEF_MAXCLIP (1 << H3D_FACEF_CLIPPLANES)
#define H3D_FACEF_CLIPLOW 0.0

// #define H3D_DBUF_NORM 0
// #define H3D_DBUF_MID 0
// #define H3D_DBUF_FAST (1 << 24)

// These aren't necessarily hard limits; that's 65536
#define H3D_OBJ_MAXVERTICES 8192
#define H3D_OBJ_MAXFACES 8192

// Better to work in multiples of 8??
#define H3D_SPRITE_FPDEPTH 16

// Convert floating point to a 16.16 integer.
#define H3D_FP16(x) ((int32_t)((x) * (1 << 16)))

// ----------------------
// Some helper functions
// ----------------------

// These helpers have non-namespaced names to make them easier to use.
// They may however collide with your own functions... IDK

#define IS2POW(x) (!(x & (x - 1)) && x)
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define CLAMP(v, min, max) (((v) < min) ? min : ((v) > max) ? max : (v))
#define eprintf(...) fprintf(stderr, __VA_ARGS__);
// NOTE: pitch = 0 means pointing straight up, this is to prevent gimbal
// lock!
#define YAWP2VEC(yaw, pitch, out)                                              \
  vec3(out, MSIN(pitch) * MSIN(yaw), MCOS(pitch), -MSIN(pitch) * MCOS(yaw));

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

// ----------------------
//  Framebuffer
// ----------------------

// The framebuffer object, which stores stuff like the 16 bit
// framebuffer, the depth buffer, etc
typedef struct {
  uint16_t *buffer;  // actual buffer (managed manually)
  mfloat_t *dbuffer; // Depth buffer, probably using w value instead of z
  uint16_t width;    // width of the framebuffer
  uint16_t height;   // height of the framebuffer
} haloo3d_fb;

// Get a value from the framebuffer at the given location
static inline uint16_t haloo3d_fb_get(haloo3d_fb *fb, int x, int y) {
  return fb->buffer[x + y * fb->width];
}

// Get a value from the depth buffer at a given location
static inline mfloat_t haloo3d_db_get(haloo3d_fb *fb, int x, int y) {
  return fb->dbuffer[x + y * fb->width];
}

// Set a value in the framebuffer at the given location
static inline void haloo3d_fb_set(haloo3d_fb *fb, int x, int y, uint16_t v) {
  fb->buffer[x + y * fb->width] = v;
}

// Set a value in the depth buffer at the given location
static inline void haloo3d_db_set(haloo3d_fb *fb, int x, int y, mfloat_t v) {
  fb->dbuffer[x + y * fb->width] = v;
}

// Get a value based on uv coordinates. Does not perform any smoothing
static inline uint16_t haloo3d_fb_getuv(haloo3d_fb *fb, mfloat_t u,
                                        mfloat_t v) {
  uint16_t x = (uint16_t)(fb->width * u) & (fb->width - 1);
  uint16_t y = (uint16_t)(fb->height * (1 - v)) & (fb->height - 1);
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

// Clear the depth buffer to the given value. For speed, the buffer may need to
// be set to different values depending on the triangle function used.
// - Regular triangle function requires 0. We use an inverse depth buffer,
//   so larger values (1/z) are actually closer
// - Fast triangle function requires something large. An interesting option
//   is the far clip value
static inline void haloo3d_fb_cleardepth(haloo3d_fb *fb) {
  //, const mfloat_t value) {
  const mfloat_t MAXBUF = 999999999.0;
  const int len = haloo3d_fb_size(fb);
  for (int i = 0; i < len; i++) {
    fb->dbuffer[i] = MAXBUF;
  }
}

static inline void haloo3d_fb_clear(haloo3d_fb *fb, const uint16_t value) {
  const int len = haloo3d_fb_size(fb);
  for (int i = 0; i < len; i++) {
    fb->buffer[i] = value;
  }
}

// ----------------------
//   Faces and objects
// ----------------------

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

// Given an object, precalc all the vertices by multiplying with the given
// perspective (or otherwise) matrix. It's up to you to make sure the
// out array has enough space to store all the vertices
int haloo3d_precalc_verts(haloo3d_obj *obj, mfloat_t *matrix, struct vec4 *out);

// A single object in a scene, linking back to a specific model. A traditional
// model-view matrix is not used; instead, a look vector is used and you call
// the lookvec function to calc the model view matrix. You can either use the
// lookvec as the raw value send to the lookat function, or you can use it
// as an offset from the model pos with a simple vector addition.
typedef struct {
  struct vec3 pos;
  struct vec3 lookvec;
  struct vec3 up;
  haloo3d_obj *model;
  haloo3d_fb *texture;
  struct vec3 *lighting; // a pointer to lighting, null for none
  struct vec3 scale;     // how big the thing should be in world
  uint16_t color;        // baseline color if textures aren't used
  uint8_t cullbackface;
} haloo3d_obj_instance;

// Set initial state for object, given a model and texture. Models are set
// to look in the default direction
void haloo3d_objin_init(haloo3d_obj_instance *obj, haloo3d_obj *model,
                        haloo3d_fb *tex);

// A function to simplify the retrieval of vertices for an instance
static inline struct vec4 *haloo3d_objin_fv(haloo3d_obj_instance *obj,
                                            haloo3d_facei face, int vert) {
  return obj->model->vertices + face[vert].posi;
}

// Generate a "static" face from precalced vertices and vtexture info,
// useful for functions which work on faces rather than triangles
static inline void haloo3d_make_facef(haloo3d_facei face, struct vec4 *verts,
                                      struct vec3 *vtextures,
                                      haloo3d_facef out) {
  out[0].pos = verts[face[0].posi];
  out[1].pos = verts[face[1].posi];
  out[2].pos = verts[face[2].posi];
  out[0].tex = vtextures[face[0].texi];
  out[1].tex = vtextures[face[1].texi];
  out[2].tex = vtextures[face[2].texi];
}

// If you're not using homogenous coordinates, this will fix depth issues
// in the triangle renderer, which expects w to be some value relating to
// perspective. TODO: eventually just fix z so it works!
static inline void haloo3d_facef_fixw(haloo3d_facef face) {
  face[0].pos.w = -face[0].pos.z + 1;
  face[1].pos.w = -face[1].pos.z + 1;
  face[2].pos.w = -face[2].pos.z + 1;
}

// ----------------------
//   Colors
// ----------------------

#define H3DC_A4(c) (((c) >> 12) & 0xF)
#define H3DC_R4(c) (((c) >> 8) & 0xF)
#define H3DC_G4(c) (((c) >> 4) & 0xF)
#define H3DC_B4(c) ((c) & 0xF)
#define H3DC_R8(c) ((((c) >> 4) & 0xF0) | 0x07)
#define H3DC_G8(c) (((c) & 0xF0) | 0x07)
#define H3DC_B8(c) ((((c) << 4) & 0xF0) | 0x07)
#define H3DC_RGB(r, g, b)                                                      \
  (0xF000 | (((r) & 0xF) << 8) | (((g) & 0xF) << 4) | ((b) & 0xF))
#define H3DC_ARGB(a, r, g, b)                                                  \
  ((((a) & 0xF) << 12) | (((r) & 0xF) << 8) | (((g) & 0xF) << 4) | ((b) & 0xF))

// "scale" a color by a given intensity. it WILL clip...
static inline uint16_t haloo3d_col_scale(uint16_t col, mfloat_t scale) {
  uint16_t r = H3DC_R4(col) * scale;
  uint16_t g = H3DC_G4(col) * scale;
  uint16_t b = H3DC_B4(col) * scale;
  return H3DC_RGB(r, g, b);
}

// "scale" a color by a discrete intensity from 0 to 256. 256 is 1.0
static inline uint16_t haloo3d_col_scalei(uint16_t col, uint16_t scale) {
  if (scale == 256) {
    return col;
  }
  uint16_t r = (H3DC_R4(col) * scale) >> 8;
  uint16_t g = (H3DC_G4(col) * scale) >> 8;
  uint16_t b = (H3DC_B4(col) * scale) >> 8;
  return H3DC_RGB(r, g, b);
}

// linear interpolate between two colors
static inline uint16_t haloo3d_col_lerp(uint16_t col1, uint16_t col2,
                                        mfloat_t t) {
  uint16_t r1 = H3DC_R4(col1);
  uint16_t g1 = H3DC_G4(col1);
  uint16_t b1 = H3DC_B4(col1);
  uint16_t r2 = H3DC_R4(col2);
  uint16_t g2 = H3DC_G4(col2);
  uint16_t b2 = H3DC_B4(col2);

  // clang-format off
  return H3DC_RGB((uint8_t)(t * (r2 - r1) + r1), 
                  (uint8_t)(t * (g2 - g1) + g1),
                  (uint8_t)(t * (b2 - b1) + b1));
  // clang-format on
}

// Blend src onto dest. Doesn't use floats, so may not be very accurate.
// Remember, src goes on top of dst.
static inline uint16_t haloo3d_col_blend(uint16_t src, uint16_t dst) {
  if ((src & 0xF000) == 0xF000) { // The basic cases, since there's only 4 bits
    return src;
  } else if ((src & 0xF000) == 0) {
    return dst;
  }
  uint16_t a1 = H3DC_A4(src);
  uint16_t r1 = H3DC_R4(src);
  uint16_t g1 = H3DC_G4(src);
  uint16_t b1 = H3DC_B4(src);

  uint16_t a2 = H3DC_A4(dst);
  uint16_t r2 = H3DC_R4(dst);
  uint16_t g2 = H3DC_G4(dst);
  uint16_t b2 = H3DC_B4(dst);

  uint8_t a = (a1 * a1 + (15 - a1) * a2) / 15;
  uint8_t r = (r1 * a1 + (15 - a1) * r2) / 15;
  uint8_t g = (g1 * a1 + (15 - a1) * g2) / 15;
  uint8_t b = (b1 * a1 + (15 - a1) * b2) / 15;

  // clang-format off
  return H3DC_ARGB(a, r, g, b);
  // clang-format on
}

// ----------------------
//   Camera
// ----------------------

// Describes a camera which can rotate in two directions only (no rolling)
typedef struct {
  struct vec3 pos;
  struct vec3 up;
  mfloat_t pitch;
  mfloat_t yaw;
} haloo3d_camera;

// Initialize the camera to look in a safe direction with
// reasonable up/etc. You spawn at the origin
void haloo3d_camera_init(haloo3d_camera *cam);

// Calculate the look vector (returned), using it to set the given matrix view
// to the "look_at" matrix
struct vec3 haloo3d_camera_calclook(haloo3d_camera *cam, mfloat_t *view);

// My personal lookat function, which does not perform the inverse or anything.
// Because of this, you can use it to orient models too
void haloo3d_my_lookat(mfloat_t *view, mfloat_t *from, mfloat_t *to,
                       mfloat_t *up);

// My personal perspective projection function. For some reason, it produces
// different results than the mathc libary's
void haloo3d_perspective(mfloat_t *m, mfloat_t fov, mfloat_t aspect,
                         mfloat_t near, mfloat_t far);

// Calculate movement based only on yaw (pretty normal)
void haloo3d_camera_calcmove_yaw(haloo3d_camera *cam, struct vec4 *delta);

// ----------------------
//   Math
// ----------------------

// Define a rectangle with INCLUSIVE start EXCLUSIVE endpoint
typedef struct {
  int x1;
  int y1;
  int x2;
  int y2;
} haloo3d_recti;

// Calculate the dimensions of the given rectangle
static inline struct vec2i haloo3d_recti_dims(haloo3d_recti *bounds) {
  return (struct vec2i){
      .x = abs(bounds->x2 - bounds->x1),
      .y = abs(bounds->y2 - bounds->y1),
  };
}

// Convert the given point to be rendered inside the given viewport.
static inline void haloo3d_viewport_into(mfloat_t *v, int width, int height) {
  // SOMETIMES we need -1, but it's because of bad programming maybe? IDK
  v[0] = round((v[0] + 1.0) / 2.0 * (width - 0));
  v[1] = round((1.0 - ((v[1] + 1.0) / 2.0)) * (height - 0));
  //  Don't touch Z or whatever
}

// Convert all points in the given face to be rendered inside the given
// viewport.
static inline void haloo3d_facef_viewport_into(haloo3d_facef face, int width,
                                               int height) {
  haloo3d_viewport_into(face[0].pos.v, width, height);
  haloo3d_viewport_into(face[1].pos.v, width, height);
  haloo3d_viewport_into(face[2].pos.v, width, height);
}

// Apply a simple scaling (this PROBABLY isn't what you want)
static inline void haloo3d_mat4_scale(mfloat_t *m, mfloat_t scale) {
  m[0] *= scale;
  m[5] *= scale;
  m[10] *= scale;
}

// Apply a simple scaling (this PROBABLY isn't what you want)
static inline void haloo3d_mat4_scalev(mfloat_t *m, mfloat_t *scale) {
  m[0] *= scale[0];
  m[5] *= scale[1];
  m[10] *= scale[2];
}

// Apply a "prescale" to some model matrix. This will apply a scale to the
// model-view matrix like you expect, this IS the function you want
static inline void haloo3d_mat4_prescalev(mfloat_t *m, mfloat_t *scale) {
  m[0] *= scale[0];
  m[1] *= scale[0];
  m[2] *= scale[0];
  m[3] *= scale[0];
  m[4] *= scale[1];
  m[5] *= scale[1];
  m[6] *= scale[1];
  m[7] *= scale[1];
  m[8] *= scale[2];
  m[9] *= scale[2];
  m[10] *= scale[2];
  m[11] *= scale[2];
  // W in the scale matrix is 1, like identity. No need to do anything
}

// Divide x, y, and z by the w value. Preserves the original w value!!
// This is often the last step of perspective projection (perspective divide)
static inline void haloo3d_vec4_conventional(struct vec4 *v) {
  if (v->w != 1) {
    v->x /= v->w;
    v->y /= v->w;
    v->z /= v->w;
  }
}

// Multiply the given point by the given matrix, returning a new point
static inline struct vec4 haloo3d_vec4_multmat(struct vec4 *v, mfloat_t *m) {
  struct vec4 result;
  result.x = v->x * m[0] + v->y * m[4] + v->z * m[8] + m[12];
  result.y = v->x * m[1] + v->y * m[5] + v->z * m[9] + m[13];
  result.z = v->x * m[2] + v->y * m[6] + v->z * m[10] + m[14];
  result.w = v->x * m[3] + v->y * m[7] + v->z * m[11] + m[15];
  return result;
}

// Multiply the given point by the given matrix, storing the result in another
// vec. It HAS to be a different vec!!
static inline void haloo3d_vec4_multmat_into(struct vec4 *v, mfloat_t *m,
                                             struct vec4 *out) {
  out->x = v->x * m[0] + v->y * m[4] + v->z * m[8] + m[12];
  out->y = v->x * m[1] + v->y * m[5] + v->z * m[9] + m[13];
  out->z = v->x * m[2] + v->y * m[6] + v->z * m[10] + m[14];
  out->w = v->x * m[3] + v->y * m[7] + v->z * m[11] + m[15];
}

// linear interpolate the two whole vectors, storing result back into first
static inline void haloo3d_vertexf_lerp_self(haloo3d_vertexf *v,
                                             haloo3d_vertexf *v2, mfloat_t t) {
  vec4_lerp(v->pos.v, v->pos.v, v2->pos.v, t);
  vec3_lerp(v->tex.v, v->tex.v, v2->tex.v, t);
}

// calculate the normal for the given face
static inline void haloo3d_facef_normal(haloo3d_facef face, mfloat_t *normal) {
  mfloat_t lt[VEC3_SIZE * 2];
  // struct vec3 l1, l2;
  vec3_subtract(lt, face[2].pos.v, face[0].pos.v);
  vec3_subtract(lt + VEC3_SIZE, face[1].pos.v, face[0].pos.v);
  vec3_cross(normal, lt, lt + VEC3_SIZE);
  vec3_normalize(normal, normal);
}

// Calculate simple light intensity for the given face when the light is facing
// the given way.
mfloat_t haloo3d_calc_light(mfloat_t *light, mfloat_t minlight,
                            haloo3d_facef face);

// ----------------------
//  Rendering
// ----------------------

// return horizontal difference per pixel for some property
#define H3D_TRIDIFF_H(v0, v1, v2, t)                                           \
  ((v0->t - v2->t) * (v1->pos.y - v2->pos.y) -                                 \
   (v1->t - v2->t) * (v0->pos.y - v2->pos.y)) /                                \
      ((v0->pos.x - v2->pos.x) * (v1->pos.y - v2->pos.y) -                     \
       (v1->pos.x - v2->pos.x) * (v0->pos.y - v2->pos.y))
// return horizontal difference per pixel for any generic value
#define H3D_TRIDIFF_HG(v0, v1, v2, t0, t1, t2)                                 \
  (((t0) - (t2)) * (v1->pos.y - v2->pos.y) -                                   \
   ((t1) - (t2)) * (v0->pos.y - v2->pos.y)) /                                  \
      ((v0->pos.x - v2->pos.x) * (v1->pos.y - v2->pos.y) -                     \
       (v1->pos.x - v2->pos.x) * (v0->pos.y - v2->pos.y))
// return vertical difference per pixel for some property
#define H3D_TRIDIFF_V(v0, v1, v2, t)                                           \
  ((v0->t - v2->t) * (v1->pos.x - v2->pos.x) -                                 \
   (v1->t - v2->t) * (v0->pos.x - v2->pos.x)) /                                \
      ((v0->pos.x - v2->pos.x) * (v1->pos.y - v2->pos.y) -                     \
       (v1->pos.x - v2->pos.x) * (v0->pos.y - v2->pos.y))

// Set dither to a 4x4 amount from 0 (no fill) to 1 (full fill).
void haloo3d_getdither4x4(float dither, uint8_t *buf);

#define H3DR_LIGHTING (1 << 0)
#define H3DR_TRANSPARENCY (1 << 1)
// DON'T USE THIS! LET THE SYSTEM CALC IT FOR YOU!
#define H3DR_TEXTURED (1 << 2)
#define H3DR_DITHERTRI (1 << 3)
#define H3DR_DITHERPIX (1 << 4)
// DON'T USE THIS! LET THE SYSTEM CALC IT FOR YOU!
#define H3DR_PCT (1 << 5)

typedef struct {
  // pointer to texture.
  haloo3d_fb *texture;
  // 1 = normal, 0 = black
  mfloat_t intensity;
  // where dithering starts and becomes full.
  mfloat_t ditherclose, ditherfar;
  // What size triangle perspective correct textures kick in. Set to
  // 0 to always have perspective correct, and like a billion or something
  // for always perspective incorrect
  uint32_t pctminsize;
  // solid color for triangle. only applies if texture null
  // uint16_t basecolor;
  // Some flags for rendering. If these aren't set, values in this
  // struct may NOT be used
  uint8_t flags;
  // 0 = none, 1 = per-triangle dither, 2 = per-pixel dither (slower)
  // uint8_t dithertype;
} haloo3d_trirender;

// uint8_t dither[8];  // 8x8 dithering (0 will make it transparent)

// Initialize a triangle render setting with all defaults
void haloo3d_trirender_init(haloo3d_trirender *tr);

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

// Draw a textured triangle into the given framebuffer using the given face.
// This function uses a "modern" technique and may be parallelized based
// on configuration or machine. It will also always be perspective correct
// and mostly accurate. If additional effects are added to the library, they
// may be applied here. It is also slow

void haloo3d_triangle(haloo3d_fb *fb, haloo3d_trirender *render,
                      haloo3d_facef face);

// Draw a textured triangle into the given framebuffer using the given face.
// This function uses an "oldschool" method for drawing triangles and is
// inherently single-threaded. It is also perspective-incorrect (like psx)
// and will not have any new features added to it. As such, it is a
// stable implementation that will most likely never change and always
// be as fast as possible.
// void haloo3d_texturedtriangle_fast(haloo3d_fb *fb, haloo3d_trirender *render,
// haloo3d_facef face);

// Draw a textured triangle into the given framebuffer using the given face.
// This function uses an "oldschool" method for drawing triangles and is
// inherently single-threaded. However, it is slower than the fast function
// because it uses floats and is perspective correct. Like the fast render
// function, it will most likely not have new features added to it
// void haloo3d_texturedtriangle_mid(haloo3d_fb *fb, haloo3d_trirender *render,
// haloo3d_facef face);

// Finalize a face, fixing xyz/w for all vertices and returning
// whether or not the triangle will be drawn.
int haloo3d_facef_finalize(haloo3d_facef face);

// Return whether a face is a backface
// int haloo3d_facef_isbackface(haloo3d_facef face);

// Clip a given face, outputting the results into the out buffer. The out
// buffer should have enough space to store all clipfaces (use
// H3D_FACEF_MAXCLIP as length). Returns the number of clipped faces.
// If 0, you can skip additional processing for this face completely
int haloo3d_facef_clip(haloo3d_facef face, haloo3d_facef *out);

// Clip a face ONLY against the z-near plane
// int haloo3d_facef_clipmin(haloo3d_facef face, haloo3d_facef *out);

// Draw a sprite with no depth value directly into the buffer. Very fast.
void haloo3d_sprite(haloo3d_fb *fb, haloo3d_fb *sprite, haloo3d_recti texrect,
                    haloo3d_recti outrect);

// Trivially fill a whole framebuffer with no transparency at integer scaling.
// Faster than haloo3d_sprite for the specific task of scaling a framebuffer
// into another. Scale is determined by the largest whole number which will fit
// each dimension. Image is centered and aspect ratio is preserved
void haloo3d_fb_fill(haloo3d_fb *dst, haloo3d_fb *src);

#endif
