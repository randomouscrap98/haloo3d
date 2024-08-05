// haloopdy 2024

#include "haloo3d.h"
#include "mathc.c"
#include <float.h>
#include <stdlib.h>
#include <string.h>

void haloo3d_fb_init(haloo3d_fb *fb, uint16_t width, uint16_t height) {
  fb->width = width;
  fb->height = height;
  mallocordie(fb->buffer, H3DFB_BSZ(fb));
  mallocordie(fb->wbuffer, H3DFB_DBSZ(fb));
}

void haloo3d_fb_free(haloo3d_fb *fb) {
  free(fb->buffer);
  free(fb->wbuffer);
}

void haloo3d_fb_cleardepth(haloo3d_fb *fb) {
  memset(fb->wbuffer, FLT_MAX, H3DFB_DBSZ(fb));
}
