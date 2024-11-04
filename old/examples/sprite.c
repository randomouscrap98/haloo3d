#include "../haloo3d.h"
#include "../haloo3dex_img.h"

#define WIDTH 640
#define HEIGHT 480
#define ITERATIONS 6000
#define OUTFILE "sprite.ppm"

int main(int argc, char **argv) {

  if (argc != 2) {
    dieerr("You must pass the image ppm to draw!\n");
  }

  // Load the junk
  haloo3d_fb tex;
  haloo3d_img_loadppmfile(&tex, argv[1]);

  // Now we create a framebuffer to draw the sprite into
  haloo3d_fb fb;
  haloo3d_fb_init(&fb, WIDTH, HEIGHT);

  for (int i = 0; i < ITERATIONS; i++) {
    haloo3d_sprite(
        &fb, &tex,
        (haloo3d_recti){.x1 = 0, .y1 = 0, .x2 = tex.width, .y2 = tex.height},
        (haloo3d_recti){.x1 = 0, .y1 = 0, .x2 = fb.width, .y2 = fb.height});
  }

  haloo3d_img_writeppmfile(&fb, OUTFILE);

  haloo3d_fb_free(&tex);
  haloo3d_fb_free(&fb);
}
