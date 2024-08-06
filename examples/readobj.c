#include "../haloo3d.h"
#include "../haloo3dex_obj.h"
#include <stdio.h>

#define READFILE "../diablo.obj"

int main(int argc, char **argv) {

  if (argc != 2) {
    dieerr("You must pass in the name of the obj to load!\n");
  }

  // Open a simple file and output the ppm to it
  FILE *f = fopen(argv[1], "r");
  if (f == NULL) {
    dieerr("Can't open %s for reading\n", argv[1]);
  }

  haloo3d_obj obj;
  haloo3d_obj_load(&obj, f);

  fclose(f);

  printf("Read from %s\n", argv[1]);

  haloo3d_obj_free(&obj);
}
