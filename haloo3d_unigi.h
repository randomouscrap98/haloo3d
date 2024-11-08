// haloopdy 2024
// An extra header you can use if you're going to use haloo3d with unigi.
// It provides a simple framebuffer with a zbuffer attached, and functions
// to manipulate the buffer

#ifndef HALOO3D_UNIGI_H
#define HALOO3D_UNIGI_H

#include "haloo3d.h"
#include <stdio.h>

// "scale" a color by a given intensity. it WILL clip...
static inline uint16_t h3d_col_scale(uint16_t col, float_t scale) {
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
static inline uint16_t h3d_col_lerp(uint16_t col1, uint16_t col2, float_t t) {
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
void h3d_img_writeppm(h3d_fb *fb, FILE *f);
// Loads a P6 binary ppm into a framebuffer
void h3d_img_loadppm(FILE *f, h3d_fb *fb);

// Write a P6 binary ppm to a file. Kills whole program if it can't
void h3d_img_writeppmfile(h3d_fb *fb, char *filename);
// Load a P6 binary ppm into the given texture. Kills whole program
// if it can't.
void h3d_img_loadppmfile(h3d_fb *tex, char *filename);

#endif
