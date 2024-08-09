#ifndef HALOO3D_PRINT_H
#define HALOO3D_PRINT_H

#include "haloo3d.h"
#include <stdarg.h>
#include <stdint.h>
#include <string.h>

#define H3D_PRINT_CHW 8
#define H3D_PRINT_CHH 8

typedef struct {
  int x;
  int y;
  int scale;
  haloo3d_fb *fb;
  char *buffer;
  int buflen;
  uint16_t bcolor;
  uint16_t fcolor;
  const uint64_t *glyphs;
  // haloo3d_fb *font;
} haloo3d_print_tracker;

// Initialize the given print tracker to have all defaults. You will still need
// to pass some kind of char buffer for storing buffered prints. The default
// colors are chosen by the library (white text on black background)
void haloo3d_print_initdefault(haloo3d_print_tracker *t, char *buf, int buflen);

// print using the given tracker. Standard printf formatter
void haloo3d_print(haloo3d_print_tracker *t, const char *fmt, ...);

// reset the cursor and any other "temporary" tracking.
void haloo3d_print_refresh(haloo3d_print_tracker *t);

// Convert a single glyph from compact uint64_t into the given framebuffer
void haloo3d_print_convertglyph(uint64_t glpyh, uint16_t bcolor,
                                uint16_t fcolor, haloo3d_fb *out);

#endif
