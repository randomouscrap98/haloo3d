#include "../haloo3d.h"
#include "../haloo3dex_gen.h"
#include "../haloo3dex_img.h"
// #include "../haloo3dex_print.h"

#define WIDTH 32
#define HEIGHT 32
#define OUTFILE "texgen.ppm"

int main() { // int argc, char **argv) {
  haloo3d_fb fb, tex;
  haloo3d_fb_init(&fb, WIDTH * 4, HEIGHT * 4);
  haloo3d_fb_init_tex(&tex, WIDTH, HEIGHT);

  haloo3d_recti texrect = {.x1 = 0, .y1 = 0, .x2 = WIDTH, .y2 = HEIGHT};
  haloo3d_recti outrect = {.x1 = 0, .y1 = 0, .x2 = WIDTH, .y2 = HEIGHT};

  // All right, let's generate a green texture
  uint16_t green = 0xF0F0;
  haloo3d_apply_alternating(&tex, &green, 1);
  haloo3d_sprite(&fb, &tex, texrect, outrect);

  // Printing to screen needs tracking
  // haloo3d_print_tracker t;
  // char printbuf[8192];
  // haloo3d_print_initdefault(&t, printbuf, sizeof(printbuf));
  // t.fb = &fb;
  // t.logprints = 1;

  haloo3d_img_writeppmfile(&fb, OUTFILE);

  haloo3d_fb_free(&fb);
  haloo3d_fb_free(&tex);
}
