
#include "haloo3d_image.h"
#include <stdio.h>

void haloo3d_writeppm(haloo3d_fb *fb, FILE *f) {
  // Do nothing yet
  fprintf(f, "P6 %d %d 255\n", fb->width, fb->height);
}
