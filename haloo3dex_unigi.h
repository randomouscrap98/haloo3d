// haloopdy 2024
// An extra header you can use if you're going to use haloo3d with unigi.
// It provides a simple framebuffer with a zbuffer attached, and functions
// to manipulate the buffer

#ifndef HALOO3D_UNIGI_H
#define HALOO3D_UNIGI_H

#include "haloo3d.h"
#include <stdio.h>

// ========================================
// |               COLOR                  |
// ========================================

// Some color conversion specific to unigi
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
static inline uint16_t haloo3d_col_scale(uint16_t col, float_t scale) {
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
                                        float_t t) {
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

// ========================================
// |            FRAMEBUFFER               |
// ========================================

// The framebuffer object, which stores stuff like the 16 bit
// framebuffer, the depth buffer, etc
typedef struct {
  uint16_t *buffer; // actual buffer (managed manually)
  float_t *dbuffer; // Depth buffer, probably using w value instead of z
  uint16_t width;   // width of the framebuffer
  uint16_t height;  // height of the framebuffer
} haloo3d_fb;

// Get a value from the framebuffer at the given location
static inline uint16_t haloo3d_fb_get(haloo3d_fb *fb, int x, int y) {
  return fb->buffer[x + y * fb->width];
}

// Get a value from the depth buffer at a given location
static inline float_t haloo3d_db_get(haloo3d_fb *fb, int x, int y) {
  return fb->dbuffer[x + y * fb->width];
}

// Set a value in the framebuffer at the given location
static inline void haloo3d_fb_set(haloo3d_fb *fb, int x, int y, uint16_t v) {
  fb->buffer[x + y * fb->width] = v;
}

// Set a value in the depth buffer at the given location
static inline void haloo3d_db_set(haloo3d_fb *fb, int x, int y, float_t v) {
  fb->dbuffer[x + y * fb->width] = v;
}

// Get a value based on uv coordinates. Does not perform any smoothing
static inline uint16_t haloo3d_fb_getuv(haloo3d_fb *fb, float_t u, float_t v) {
  // NOTE: Some multiplications here have been changed for systems
  // where float constants are slow for some reason. For instance, we
  // multiplied out height with 1 - v
  uint16_t x = (uint16_t)(fb->width * u) & (fb->width - 1);
  uint16_t y = (uint16_t)(fb->height - fb->height * v) & (fb->height - 1);
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

// ===========================================
// |                 IMAGE                   |
// ===========================================

// Writes a P6 binary ppm from the framebuffer
void haloo3d_image_writeppm(haloo3d_fb *fb, FILE *f);
// Loads a P6 binary ppm into a framebuffer
void haloo3d_image_loadppm(FILE *f, haloo3d_fb *fb);

// Convert given color to transparent in framebuffer
void haloo3d_image_totransparent(haloo3d_fb *fb, uint16_t col);

// Write a P6 binary ppm to a file. Kills whole program if it can't
void haloo3d_image_writeppmfile(haloo3d_fb *fb, char *filename);
// Load a P6 binary ppm into the given texture. Kills whole program
// if it can't.
void haloo3d_image_loadppmfile(haloo3d_fb *tex, char *filename);

#endif
