
#include "haloo3dex_img.h"
#include <stdio.h>

void haloo3d_writeppm(haloo3d_fb *fb, FILE *f) {
  fprintf(f, "P6 %d %d 255\n", fb->width, fb->height);
  uint8_t color[3];
  for (size_t i = 0; i < haloo3d_fb_size(fb); i++) {
    uint16_t bc = fb->buffer[i];
    color[0] = H3DC_R(bc);
    color[1] = H3DC_G(bc);
    color[2] = H3DC_B(bc);
    fwrite(color, sizeof(uint8_t), 3, f);
  }
}
