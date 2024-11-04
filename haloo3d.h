#ifndef __HALOO3D_HEADER
#define __HALOO3D_HEADER

// I don't know, for now I'm going to use this.
// #include "haloo3dex_helper.h"
#include <stdint.h>

typedef float float_t;

// Some systems get caught up on float literals for some reason...?
#ifdef H3D_VOLATILE_FLOATS
#define H3DVF(x) (volatile mfloat_t)(x)
#else
#define H3DVF(x) (x)
#endif

// All vector types and such are simply arrays of floats.
// These are indexes into the arrays for the various values
#define H3DX 0
#define H3DY 1
#define H3DZ 2
#define H3DW 3

#define H3D_MAXINTERPOLANTS 8

// ================================================
// |                  MATH                        |
// ================================================

// Edge function for a point being on one side or another of a line given by
// v0 and v1. We use counter-clockwise winding. Technically returns 2 * area
// of triangle created by v0, v1, and p.
#define H3D_EDGEFUNC(v0, v1, p)                                                \
  (p[H3DX] - v0[H3DX]) * (v1[H3DY] - v0[H3DY]) -                               \
      (p[H3DY] - v0[H3DY]) * (v1[H3DX] - v0[H3DX])

// return horizontal difference per pixel for any generic value in vertex
#define H3D_HLERP(v0, v1, v2, t0, t1, t2)                                      \
  (((t0) - (t2)) * (v1->pos[H3DY] - v2->pos[H3DY]) -                           \
   ((t1) - (t2)) * (v0->pos[H3DY] - v2->pos[H3DY])) /                          \
      ((v0->pos[H3DX] - v2->pos[H3DX]) * (v1->pos[H3DY] - v2->pos[H3DY]) -     \
       (v1->pos[H3DX] - v2->pos[H3DX]) * (v0->pos[H3DY] - v2->pos[H3DY]))

// ================================================
// |                  RASTER                      |
// ================================================

// A single vertex for rasterization. Vertices are 2d and only used for
// rasterization
typedef struct {
  int16_t pos[2];
  float_t interpolants[H3D_MAXINTERPOLANTS];
} h3d_rastervertex;

// Represents tracking information for one side of a triangle.
// It may not use or calculate all fields; the left sides require
// the interpolation and the right side only needs x. It uses a
// stack of vectors setup by determining the "direction" of the
// scanline; the "stack" is popped when a vector section is complete
typedef struct {
  h3d_rastervertex *stack[3]; // Vertex order
  int top;
  int sectionheight; // Tracking for how much is left in the current section
  float_t x, x_dy;   // Tracking for specifically the x coordinate.
  float_t interpolants[H3D_MAXINTERPOLANTS];   // Tracking variables
  float_t interpolant_dy[H3D_MAXINTERPOLANTS]; // Y Delta along current edge
  uint8_t num_interpolants;
} _h3dtriside;

// Initialize h3dtriside struct for tracking variables along edge of tri
static inline void _h3dtriside_init(_h3dtriside *s, uint8_t num_interpolants) {
  s->top = 0;
  s->sectionheight = 0;
  s->num_interpolants = num_interpolants;
}

// Push a raster vector onto the vector stack. Remember it is a stack,
// so the ones you want on top should go last
static inline int _h3dtriside_push(_h3dtriside *s, h3d_rastervertex *v) {
  s->stack[s->top] = v;
  return ++s->top;
}

// Pop a vector, returning the new top
static inline int _h3dtriside_pop(_h3dtriside *s) { return --s->top; }

// Calculate all deltas and return the height of this section.
static inline int _h3dtriside_start(_h3dtriside *s) {
  const h3d_rastervertex *const v1 = s->stack[s->top - 1];
  const h3d_rastervertex *const v2 = s->stack[s->top - 2];
  const int height = v2->pos[H3DY] - v1->pos[H3DY];
  if (height == 0) {
    return 0;
  }
  const float_t invheight = H3DVF(1.0) / height;
  s->x_dy = (v2->pos[H3DX] - v1->pos[H3DX]) * invheight;
  s->x = v1->pos[H3DX];
  // eprintf("X and x_dy: %f %f\n", s->x, s->x_dy);
  // Copy interpolants from v1, the top of the stack (and current line)
  for (int i = 0; i < s->num_interpolants; i++) {
    s->interpolants[i] = v1->interpolants[i];
    // Your interpolants need to be linear across the triangle, otherwise
    // this doesn't work!
    s->interpolant_dy[i] =
        (v2->interpolants[i] - v1->interpolants[i]) * invheight;
  }
  s->sectionheight = height;
  return height;
}

// Move to the next line along the side. Returns 1 if the
// entire side is done
static inline int _h3dtriside_next(_h3dtriside *s) {
  // At bottom of current section
  if (--s->sectionheight <= 0) {
    // There needs to be at least two vertices to work
    if (_h3dtriside_pop(s) < 2) {
      return 1;
    }
    // The next section has no height. Note that if this succeeds,
    // this begins the next section, and thus no recalcs are needed
    if (_h3dtriside_start(s) <= 0) {
      return 1;
    }
  } else {
    s->x += s->x_dy;
    for (int i = 0; i < s->num_interpolants; i++) {
      s->interpolants[i] += s->interpolant_dy[i];
    }
  }
  return 0;
}

// TODO: remember to invert v for linear interpolation?
// TODO: remember to make textures wrap to prevent buffer overflow
// TODO: dithering, scaled intensity per tri, etc.

#define _H3D_CLAMP(v, min, max) (((v) < min) ? min : ((v) > max) ? max : (v))

// Clamp the values. This may be wasteful but it's safer...
// - rv = rasterize vertex
// - bw = buffer width
// - bh = buffer height
#define H3DTRI_CLAMP(rv, bw, bh)                                               \
  for (int _i = 0; _i < 3; _i++) {                                             \
    rv[_i].pos[H3DX] = _H3D_CLAMP(rv[_i].pos[H3DX], H3DVF(0), bw - H3DVF(1));  \
    rv[_i].pos[H3DY] = _H3D_CLAMP(rv[_i].pos[H3DY], H3DVF(0), bh - H3DVF(1));  \
  }

// Helper function for initializing triangle functions. This enables
// shader-like features.
// - rv = initial raster vector,
// - sv = how to name the sorted vector pointer array,
// - parea = how to name variable for calculating the edge-function 2xarea
//   of the tri (useful for backface culling)
#define H3DTRI_BEGIN(rv, sv, parea)                                            \
  h3d_rastervertex *sv[3];                                                     \
  for (int _i = 0; _i < 3; _i++) {                                             \
    sv[_i] = &rv[_i];                                                          \
  }                                                                            \
  /* Make sure vertices are sorted top to bottom */                            \
  if (sv[0]->pos[H3DY] > sv[1]->pos[H3DY]) {                                   \
    h3d_rastervertex *tmp = sv[0];                                             \
    sv[0] = sv[1];                                                             \
    sv[1] = tmp;                                                               \
  }                                                                            \
  if (sv[1]->pos[H3DY] > sv[2]->pos[H3DY]) {                                   \
    h3d_rastervertex *tmp = sv[1];                                             \
    sv[1] = sv[2];                                                             \
    sv[2] = tmp;                                                               \
  }                                                                            \
  if (sv[0]->pos[H3DY] > sv[1]->pos[H3DY]) {                                   \
    h3d_rastervertex *tmp = sv[0];                                             \
    sv[0] = sv[1];                                                             \
    sv[1] = tmp;                                                               \
  }                                                                            \
  int32_t parea = H3D_EDGEFUNC(sv[0]->pos, sv[1]->pos, sv[2]->pos);

// eprintf("order: (%d,%d) (%d,%d) (%d,%d)\n", sv[0]->pos[0], sv[0]->pos[1],
//         sv[1]->pos[0], sv[1]->pos[1], sv[2]->pos[0], sv[2]->pos[1]);

// Helper macro for beginning the triangle loop. After this, create your
// "shader" (inner loop)
// - sv = sorted vertex pointer array
// - parea = parea calculated in H3DTRI_BEGIN
// - linit = name of the linear array (you will use this in shader)
// - numlinit = number of linear values
// - bw = buffer width
// - bh = buffer height
// - bufi = name of buffer index variable (you will use this in shder)
#define H3DTRI_SCAN_BEGIN(sv, parea, linit, numlinit, bw, bh, bufi)            \
  /* Tracking info for left and right side. Only tracks vertical strides */    \
  _h3dtriside _right, _left;                                                   \
  _h3dtriside_init(&_right, numlinit);                                         \
  _h3dtriside_init(&_left, numlinit);                                          \
  { /* Scoped pointers */                                                      \
    _h3dtriside *onesec, *twosec;                                              \
    if (parea == 0) {                                                          \
      return;                                                                  \
    } else if (parea < 0) {                                                    \
      /* The middle point is on the right side, because it's wound clockwise*/ \
      parea = -parea;                                                          \
      onesec = &_left;                                                         \
      twosec = &_right;                                                        \
    } else {                                                                   \
      /* The middle point is on the left side, as expected for our winding */  \
      onesec = &_right;                                                        \
      twosec = &_left;                                                         \
    }                                                                          \
    _h3dtriside_push(twosec, sv[2]);                                           \
    _h3dtriside_push(twosec, sv[1]);                                           \
    _h3dtriside_push(twosec, sv[0]);                                           \
    _h3dtriside_push(onesec, sv[2]);                                           \
    _h3dtriside_push(onesec, sv[0]);                                           \
    /* Calculate the deltas. If the "onesection" side has no height, we die */ \
    if (_h3dtriside_start(onesec) <= 0) {                                      \
      return;                                                                  \
    }                                                                          \
    if (_h3dtriside_start(twosec) <= 0) {                                      \
      /* The "twosection" side has another section. */                         \
      _h3dtriside_pop(twosec);                                                 \
      if (_h3dtriside_start(twosec) <= 0) {                                    \
        return;                                                                \
      }                                                                        \
    }                                                                          \
  }                                                                            \
  /* need to calc all the constant horizontal diffs. */                        \
  float_t _dx[H3D_MAXINTERPOLANTS];                                            \
  float_t linit[H3D_MAXINTERPOLANTS];                                          \
  for (int _i = 0; _i < numlinit; _i++) {                                      \
    _dx[_i] = H3D_HLERP(sv[0], sv[1], sv[2], sv[0]->interpolants[_i],          \
                        sv[1]->interpolants[_i], sv[2]->interpolants[_i]);     \
  }                                                                            \
  uint16_t _y = sv[0]->pos[H3DY];                                              \
  uint32_t _bufstart = (uint32_t)_y * bw;                                      \
  while (1) {                                                                  \
    /* Supposed to use ceiling but idk, mine works with floor... kinda. */     \
    /* I have holes but with ceil I get actual seams. */                       \
    uint16_t _xl = _left.x;                                                    \
    uint16_t _xr = _right.x;                                                   \
    if (_xl < _xr) {                                                           \
      float_t xofs = _xl - _left.x;                                            \
      /* Setup interpolants for inner loop, shifting by appropriate amount*/   \
      for (int _i = 0; _i < numlinit; _i++) {                                  \
        linit[_i] = _left.interpolants[_i] + xofs * _dx[_i];                   \
      }                                                                        \
      for (uint32_t bufi = _bufstart + _xl; bufi < _bufstart + _xr; bufi++)

// eprintf("xl: %d xr: %d\n", _xl, _xr);

// Optimized macros you need to call in your inner loop (shader) to update
// the linear values
#define H3DTRI_LINIT1(linit) linit[0] += _dx[0];
#define H3DTRI_LINIT2(linit)                                                   \
  linit[0] += _dx[0];                                                          \
  linit[1] += _dx[1];
#define H3DTRI_LINIT3(linit)                                                   \
  linit[0] += _dx[0];                                                          \
  linit[1] += _dx[1];                                                          \
  linit[2] += _dx[2];
#define H3DTRI_LINIT4(linit)                                                   \
  linit[0] += _dx[0];                                                          \
  linit[1] += _dx[1];                                                          \
  linit[2] += _dx[2];                                                          \
  linit[3] += _dx[3];

// end the loop created by H3D_SCAN_BEGIN
#define H3DTRI_SCAN_END()                                                      \
  }                                                                            \
  _y++;                                                                        \
  _bufstart += bw;                                                             \
  if (_h3dtriside_next(&_left)) {                                              \
    return;                                                                    \
  }                                                                            \
  if (_h3dtriside_next(&_right)) {                                             \
    return;                                                                    \
  }                                                                            \
  }

#endif
