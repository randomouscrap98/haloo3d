#ifndef __HALOO3D_FB_H
#define __HALOO3D_FB_H

#include "haloo3d.h"

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
} h3d_fb;

typedef struct {
  uint32_t *buffer; // actual buffer (managed manually)
  float_t *dbuffer; // Depth buffer, probably using w value instead of z
  uint16_t width;   // width of the framebuffer
  uint16_t height;  // height of the framebuffer
} h3d_fb32;

#define H3D_FB_GET(fb, x, y) (fb->buffer[(x) + (y) * fb->width])
#define H3D_FB_DGET(fb, x, y) (fb->dbuffer[(x) + (y) * fb->width])
#define H3D_FB_SET(fb, x, y, v) fb->buffer[(x) + (y) * fb->width] = (v)
#define H3D_FB_DSET(fb, x, y, v) fb->dbuffer[x + (y) * fb->width] = (v)
#define H3D_FB_GETUV(fb, u, v)                                                 \
  (fb->buffer[((uint16_t)(fb->width * u) & (fb->width - 1)) +                  \
              ((uint16_t)(fb->height * (H3DVF(1) - v)) & (fb->height - 1)) *   \
                  fb->width])
#define H3D_FB_SIZE(fb) (fb->width * fb->height)

#endif
