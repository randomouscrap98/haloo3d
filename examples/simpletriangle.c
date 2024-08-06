#include "../haloo3d.h"
#include "../haloo3dex_img.h"
#include <stdio.h>

#define OUTFILE "simpletriangle.ppm"

int main(int argc, char **argv) {

  if (argc != 2) {
    dieerr(
        "You must pass in the name of the image to load (must be p6 ppm)!\n");
  }

  // Open a simple file and read the ppm from it
  FILE *f = fopen(argv[1], "r");
  if (f == NULL) {
    dieerr("Can't open %s for reading\n", argv[1]);
  }

  haloo3d_fb tex;
  haloo3d_loadppm(f, &tex); // This also calls init so you have to free

  printf("Read from %s\n", argv[1]);
  fclose(f);

  // Now we create a framebuffer to draw the triangle into
  haloo3d_fb fb;
  haloo3d_fb_init(&fb, 256, 256); // This also calls init so you have to free

  // Need to create a face representing the triangle
  haloo3d_facef face;
  face[0].pos.x =
      10; // Remember to use correct winding order (counter-clockwise)
  face[0].pos.y = 10;
  face[1].pos.x = 50;
  face[1].pos.y = 200;
  face[2].pos.x = 100;
  face[2].pos.y = 100;
  face[0].tex.x = 0;
  face[0].tex.y = 0;
  face[1].tex.x = 1;
  face[1].tex.y = 0;
  face[1].tex.x = 1;
  face[1].tex.y = 1;

  haloo3d_texturedtriangle(&fb, &tex, 1.0, face);

  // And now we should be able to save the framebuffer
  FILE *fo = fopen(OUTFILE, "w");
  if (f == NULL) {
    dieerr("Can't open %s for writing\n", OUTFILE);
  }

  haloo3d_writeppm(&fb, fo);

  printf("Wrote to %s\n", OUTFILE);
  fclose(fo);

  haloo3d_fb_free(&tex);
  haloo3d_fb_free(&fb);
}
