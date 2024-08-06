#include "../haloo3d.h"
#include "../haloo3dex_img.h"
#include <stdio.h>

#define OUTFILE "ppmloadstore.ppm"

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

  // And now we should be able to save the texture and it should look the same
  // (but with the color
  FILE *fo = fopen(OUTFILE, "w");
  if (f == NULL) {
    dieerr("Can't open %s for writing\n", OUTFILE);
  }

  haloo3d_writeppm(&tex, fo);

  printf("Wrote to %s\n", OUTFILE);
  fclose(fo);

  haloo3d_fb_free(&tex);
}
