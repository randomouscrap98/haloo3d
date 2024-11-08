#include "../haloo3d.h"
#include "../haloo3d_ex.h"
#include "../haloo3d_unigi.h"

#define WIDTH 160
#define HEIGHT 160
#define OUTFILE "t5_print.ppm"
#define BUFLEN 8192
#define REPEAT 10000

int main(int argc, char **argv) {

  eprintf("Program start\n");

  if (argc != 3) {
    dieerr("You must pass in the scale and text to print!\n");
  }

  // Initialize a print tracker and framebuffer to hold it
  h3d_fb fb;
  h3d_fb_init(&fb, WIDTH, HEIGHT);
  H3D_FB_FILL(&fb, 0);
  eprintf("FB init\n");

  char buffer[BUFLEN];
  h3d_print_tracker pt;
  h3d_print_init(&pt, buffer, BUFLEN, &fb);
  pt.scale = atoi(argv[1]);
  eprintf("PT init\n");

  // Do the work
  for (int i = 0; i < REPEAT; i++) {
    h3d_print_refresh(&pt);
    h3d_print(&pt, argv[2]);
  }
  h3d_fb_writeppmfile(&fb, OUTFILE);
  h3d_fb_free(&fb);
  eprintf("Done!\n");
}
