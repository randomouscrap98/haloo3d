// haloopdy 2024
// An extra header you can use if you're going to use haloo3d with unigi.
// Depends on the extended library.

#ifndef HALOO3D_UNIGI_H
#define HALOO3D_UNIGI_H

#include "haloo3d.h"
#include "haloo3d_obj.h"

#include <stdarg.h>
#include <stdio.h>
#include <time.h>

#define H3D_EASYSTORE_MAX 1024
#define H3D_EASYSTORE_MAXKEY 16

// ========================================
// |               COLOR                  |
// ========================================

// unigi-specific color operations

// "scale" a color by a given intensity. it WILL clip...
static inline uint16_t h3d_col_scale(uint16_t col, hfloat_t scale) {
  uint16_t r = H3DC_R4(col) * scale;
  uint16_t g = H3DC_G4(col) * scale;
  uint16_t b = H3DC_B4(col) * scale;
  return H3DC_A4R4G4B4(0xF, r, g, b);
}

// "scale" a color by a discrete intensity from 0 to 256. 256 is 1.0
static inline uint16_t h3d_col_scalei(uint16_t col, uint16_t scale) {
  if (scale == 256) {
    return col;
  }
  uint16_t r = (H3DC_R4(col) * scale) >> 8;
  uint16_t g = (H3DC_G4(col) * scale) >> 8;
  uint16_t b = (H3DC_B4(col) * scale) >> 8;
  return H3DC_A4R4G4B4(0xF, r, g, b);
}

// linear interpolate between two colors
static inline uint16_t h3d_col_lerp(uint16_t col1, uint16_t col2, hfloat_t t) {
  uint16_t r1 = H3DC_R4(col1);
  uint16_t g1 = H3DC_G4(col1);
  uint16_t b1 = H3DC_B4(col1);
  uint16_t r2 = H3DC_R4(col2);
  uint16_t g2 = H3DC_G4(col2);
  uint16_t b2 = H3DC_B4(col2);

  // clang-format off
  return H3DC_A4R4G4B4(
      0xF,
      (uint8_t)(t * (r2 - r1) + r1), 
      (uint8_t)(t * (g2 - g1) + g1),
      (uint8_t)(t * (b2 - b1) + b1));
  // clang-format on
}

// Blend src onto dest. Doesn't use floats, so may not be very accurate.
// Remember, src goes on top of dst.
static inline uint16_t h3d_col_blend(uint16_t src, uint16_t dst) {
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
  return H3DC_A4R4G4B4(a, r, g, b);
  // clang-format on
}

// ===========================================
// |                 IMAGE                   |
// ===========================================

// Writes a P6 binary ppm from the framebuffer
void h3d_fb_writeppm(h3d_fb *fb, FILE *f);
// Loads a P6 binary ppm into a framebuffer
void h3d_fb_loadppm(FILE *f, h3d_fb *fb);
// Write a P6 binary ppm to a file. Kills whole program if it can't
void h3d_fb_writeppmfile(h3d_fb *fb, char *filename);
// Load a P6 binary ppm into the given texture. Kills whole program
// if it can't.
void h3d_fb_loadppmfile(h3d_fb *tex, char *filename);

// ===========================================
// |              FRAMEBUFFER                |
// ===========================================

void h3d_fb_init(h3d_fb *fb, uint16_t width, uint16_t height);
void h3d_fb_free(h3d_fb *fb);

void h3d_fb_intscale(h3d_fb *src, h3d_fb *dst, int dstofsx, int dstofsy,
                     uint8_t scale);
void h3d_fb_fill(h3d_fb *src, h3d_fb *dst, uint8_t centered);

// ===========================================
// |              EASYSYS                    |
// ===========================================

// A storage container for easy access to models and textures by name.
// Adds overhead compared to direct access of models and textures.
typedef struct {
  h3d_obj _objects[H3D_EASYSTORE_MAX];
  h3d_fb _textures[H3D_EASYSTORE_MAX];
  char objkeys[H3D_EASYSTORE_MAX][H3D_EASYSTORE_MAXKEY];
  char texkeys[H3D_EASYSTORE_MAX][H3D_EASYSTORE_MAXKEY];
} h3d_easystore;

// Initialize storage unit
void h3d_easystore_init(h3d_easystore *s);

h3d_obj *h3d_easystore_addobj(h3d_easystore *s, const char *key);
h3d_obj *h3d_easystore_getobj(h3d_easystore *s, const char *key);
void h3d_easystore_deleteobj(h3d_easystore *s, const char *key,
                             void (*ondelete)(h3d_obj *));
void h3d_easystore_deleteallobj(h3d_easystore *s, void (*ondelete)(h3d_obj *));

h3d_fb *h3d_easystore_addtex(h3d_easystore *s, const char *key);
h3d_fb *h3d_easystore_gettex(h3d_easystore *s, const char *key);
void h3d_easystore_deletetex(h3d_easystore *s, const char *key,
                             void (*ondelete)(h3d_fb *));
void h3d_easystore_deletealltex(h3d_easystore *s, void (*ondelete)(h3d_fb *));

// System for tracking time. Values measured in seconds. DON'T
// expect the 'start' variable to be of any particular type; it
// will depend on the implementation details
typedef struct {
  clock_t start;
  float sum;
  float last;
  float avgweight;
  float min;
  float max;
} h3d_easytimer;

void h3d_easytimer_init(h3d_easytimer *t, float avgweight);
void h3d_easytimer_start(h3d_easytimer *t);
void h3d_easytimer_end(h3d_easytimer *t);

// ===========================================
// |              GENERATION                 |
// ===========================================

int h3d_4x4dither_index(float dither);
void h3d_getdither4x4(float dither, uint8_t *buf);

void h3d_apply_alternating(h3d_fb *fb, uint16_t *cols, uint16_t numcols);
void h3d_apply_vgradient(h3d_fb *fb, uint16_t top, uint16_t bottom);
void h3d_apply_brick(h3d_fb *fb, uint16_t width, uint16_t height,
                     uint16_t color);
void h3d_apply_rect(h3d_fb *fb, h3d_recti rect, uint16_t color, int width);
void h3d_apply_fillrect(h3d_fb *fb, h3d_recti rect, uint16_t color,
                        uint8_t dithering[4]);
void h3d_gen_solidtex(h3d_fb *fb, uint16_t color);
void h3d_gen_palettetex(h3d_fb *fb);
void h3d_gen_paletteuv(uint16_t col, vec3 result);
void h3d_gen_boxvtexture(vec3 *textures);
void h3d_gen_skybox(h3d_obj *obj);
void h3d_gen_plane(h3d_obj *obj, uint16_t size);
void h3d_gen_grid_quad(h3d_obj *obj, int x, int y, int32_t dir[2]);
void h3d_gen_grid(h3d_obj *obj, uint16_t size, uint8_t faces);
void h3d_gen_sloped(h3d_obj *obj, uint16_t size, hfloat_t slopiness,
                    hfloat_t downbias);
void h3d_gen_crossquad_generic(h3d_obj *obj, h3d_fb *fb, vec3 center,
                               int count);
void h3d_gen_crossquad(h3d_obj *obj, h3d_fb *fb, vec3 center);
void h3d_gen_quad(h3d_obj *obj, h3d_fb *fb, vec3 center);
void h3d_gen_gradient(h3d_fb *buf, uint16_t topcol, uint16_t botcol,
                      int height);

// ===========================================
// |          PRINTING (legacy)              |
// ===========================================

#define H3D_PRINT_CHW 8
#define H3D_PRINT_CHH 8

typedef struct {
  int x;
  int y;
  int scale;
  int logprints;
  h3d_fb *fb;
  char *buffer;
  int buflen;
  uint16_t bcolor;
  uint16_t fcolor;
  const uint64_t *glyphs;
  h3d_recti bounds;
  // haloo3d_fb *font;
} h3d_print_tracker;

// Initialize the given print tracker to have all defaults. You will still need
// to pass some kind of char buffer for storing buffered prints. The default
// colors are chosen by the library (white text on black background)
void h3d_print_init(h3d_print_tracker *t, char *buf, int buflen, h3d_fb *fb);

// print using the given tracker. Standard printf formatter
void h3d_print(h3d_print_tracker *t, const char *fmt, ...);

// reset the cursor and any other "temporary" tracking.
void h3d_print_refresh(h3d_print_tracker *t);

// Convert a single glyph from compact uint64_t into the given framebuffer
void h3d_print_convertglyph(uint64_t glpyh, uint16_t bcolor, uint16_t fcolor,
                            h3d_fb *out);
#endif
