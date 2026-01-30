#ifndef __RENDER_EFFECTS_H__
#define __RENDER_EFFECTS_H__

#include "../../haloo3d.h"

// Darkens given v by amount a IN PLACE. bright should be a uint8_t
// between 0 and 15, with 15 being full brightness
#define EFFECT_DARKEN_A4R4G4B4(v, bright) { \
  uint8_t a = H3DC_A4(v); \
  uint8_t r = (H3DC_R4(v) * bright) >> 4; \
  uint8_t g = (H3DC_G4(v) * bright) >> 4; \
  uint8_t b = (H3DC_B4(v) * bright) >> 4; \
  v = H3DC_A4R4G4B4(a, r, g, b); \
}

#endif
