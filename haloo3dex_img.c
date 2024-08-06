
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

void haloo3d_loadppm(FILE *f, haloo3d_fb *fb) {
  int depth;
  int scanned = fscanf(f, "P6 %hu %hu %d", &fb->width, &fb->height, &depth);
  if (scanned != 3) {
    dieerr("Image file not in P6 format");
  }
  haloo3d_fb_init_tex(fb, fb->width, fb->height);
  // Consume one character, it's the whitespace after depth
  fgetc(f);
  // Now let's just read until the end!
  int b = 0;
  int i = 0;
  int c = 0;
  while ((c = fgetc(f)) != EOF) {
    fb->buffer[i] |= 0xF000 | (uint16_t)((c / (float)depth) * 15) << (b * 4);
    b++;
    if (b == 3) { // We've read the full rgb
      i++;
      b = 0;
    }
  }
}
