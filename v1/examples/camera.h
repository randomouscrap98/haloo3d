#ifndef _HEADER_CAMERAH
#define _HEADER_CAMERAH
#include "../haloo3d.h"

typedef struct {
  mfloat_t xofs;
  mfloat_t yofs;
  mfloat_t zofs;
  mfloat_t yaw;
  mfloat_t pitch;
} camset;

// Read a camera file, producing a list of structs representing camera position
// and rotation
static int readcam(camset *set, int max, char *filename) {
  FILE *f = fopen(filename, "r");
  if (f == NULL) {
    dieerr("Can't open %s for reading cam\n", filename);
  }
  int num = 0;
  while (5 == fscanf(f, "%f %f %f %f %f", &set[num].xofs, &set[num].yofs,
                     &set[num].zofs, &set[num].yaw, &set[num].pitch)) {
    // char linebuf[1024];
    //  fgets(linebuf, sizeof(linebuf), f)) {
    num++;
    if (num >= max) {
      eprintf("Camera file too big! Ignoring rest\n");
      break;
    }
  }
  fclose(f);
  printf("Read %d camlines from %s\n", num, filename);
  return num;
}

#endif
