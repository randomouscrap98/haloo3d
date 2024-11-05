
#include "haloo3dex_img.h"
#include <stdio.h>
#include <string.h>

void haloo3d_img_writeppm(haloo3d_fb *fb, FILE *f) {
  fprintf(f, "P6 %d %d 15\n", fb->width, fb->height);
  uint8_t color[3];
  for (int i = 0; i < haloo3d_fb_size(fb); i++) {
    uint16_t bc = fb->buffer[i];
    color[0] = (bc >> 8) & 0xF; // H3DC_R(bc);
    color[1] = (bc >> 4) & 0xf; // H3DC_G(bc);
    color[2] = bc & 0xf;        // H3DC_B(bc);
    fwrite(color, sizeof(uint8_t), 3, f);
  }
}

void haloo3d_img_loadppm(FILE *f, haloo3d_fb *fb) {
  char tmp[4096];
  // Must ALWAYS start with "P6"
  int scanned = fscanf(f, "%4095s", tmp);
  if (scanned != 1 || strcmp(tmp, "P6") != 0) {
    dieerr("Image file not in P6 format (no P6 identifier)");
  }
  // Now just pull three digits
  int vals[3];
  int numvals = 0;
  while (numvals != 3) {
    scanned = fscanf(f, "%d", vals + numvals);
    if (scanned != 1) {
      // This might just be a comment. Consume the rest of the line if so
      scanned = fscanf(f, "%4095s", tmp);
      if (scanned != 1 || tmp[0] != '#' || !fgets(tmp, 4095, f)) {
        dieerr("Image file not in P6 format (unexpected header value: %s)",
               tmp);
      }
    } else {
      numvals++;
    }
  }
  // Consume one character, it's the whitespace after depth
  fgetc(f);
  fb->width = vals[0];
  fb->height = vals[1];
  int depth = vals[2];
  haloo3d_fb_init_tex(fb, fb->width, fb->height);
  // Must set everything to 0
  memset(fb->buffer, 0, haloo3d_fb_size(fb));
  // Now let's just read until the end!
  int b = 0;
  int i = 0;
  int c = 0;
  while ((c = fgetc(f)) != EOF) {
    fb->buffer[i] |=
        0xF000 | ((uint16_t)((c / (float)depth) * 15 + 0.5) << ((2 - b) * 4));
    b++;
    if (b == 3) { // We've read the full rgb
      i++;
      b = 0;
    }
  }
}

void haloo3d_img_totransparent(haloo3d_fb *fb, uint16_t col) {
  const int size = haloo3d_fb_size(fb);
  for (int i = 0; i < size; i++) {
    if (fb->buffer[i] == col)
      fb->buffer[i] = 0;
  }
}

void haloo3d_img_writeppmfile(haloo3d_fb *fb, char *filename) {
  // And now we should be able to save the framebuffer
  FILE *f = fopen(filename, "w");
  if (f == NULL) {
    dieerr("Can't open %s for writing ppm image\n", filename);
  }
  haloo3d_img_writeppm(fb, f);
  fclose(f);
  eprintf("Wrote ppm image to %s\n", filename);
}

void haloo3d_img_loadppmfile(haloo3d_fb *tex, char *filename) {
  // Open a simple file and read the ppm from it
  FILE *f = fopen(filename, "r");
  if (f == NULL) {
    dieerr("Can't open %s for ppm image reading\n", filename);
  }
  haloo3d_img_loadppm(f, tex); // This also calls init so you have to free
  fclose(f);
  eprintf("Read ppm image from %s\n", filename);
}
