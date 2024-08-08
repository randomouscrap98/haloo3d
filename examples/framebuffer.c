#include "../haloo3d.h"
#include "../haloo3dex_img.h"
#include <stdio.h>

// The ARGB format has 4096 available colors. This fits in a nice
// square of 64x64
#define WIDTH 64
#define HEIGHT 64
#define OUTFILE "framebuffer.ppm"

int main() {
  haloo3d_fb fb;
  haloo3d_fb_init(&fb, WIDTH, HEIGHT);

  // Fill framebuffer with all the colors
  for (int i = 0; i < haloo3d_fb_size(&fb); i++) {
    fb.buffer[i] = 0xF000 | i;
  }

  // Open a simple file and output the ppm to it
  FILE *f = fopen(OUTFILE, "w");
  if (f == NULL) {
    dieerr("Can't open %s for writing\n", OUTFILE);
  }

  haloo3d_img_writeppm(&fb, f);

  fclose(f);

  printf("Wrote to %s\n", OUTFILE);

  haloo3d_fb_free(&fb);
}
