#include "../haloo3d.h"
#include "../haloo3dex_img.h"
#include "../haloo3dex_print.h"

#define WIDTH 640
#define HEIGHT 480
#define ITERATIONS 6000
#define OUTFILE "print.ppm"

int main(int argc, char **argv) {

  if (argc != 3) {
    dieerr("You must pass the text to draw and the scale!\n");
  }

  char *text = argv[1];

  // Now we create a framebuffer to draw the sprite into
  haloo3d_fb fb;
  haloo3d_fb_init(&fb, WIDTH, HEIGHT);

  // Fill the buffer with something silly, like blue
  for (int i = 0; i < WIDTH * HEIGHT; i++)
    fb.buffer[i] = 0xF00F;

  // Initialize the print system with defaults.
  haloo3d_print_tracker t;
  char printbuf[8192];
  haloo3d_print_initdefault(&t, printbuf, sizeof(printbuf));
  t.scale = atoi(argv[2]);
  t.fb = &fb;

  for (int i = 0; i < ITERATIONS; i++) {
    haloo3d_print_refresh(&t);
    haloo3d_print(&t, text);
  }

  haloo3d_img_writeppmfile(&fb, OUTFILE);
  haloo3d_fb_free(&fb);
}
