#include "../haloo3d.h"
#include "../haloo3dex_img.h"
#include "../haloo3dex_obj.h"
#include <stdio.h>

#define WIDTH 512
#define HEIGHT 512
#define ITERATIONS 100
#define OUTFILE "orthographic.ppm"

void load_texture(haloo3d_fb *tex, char *filename) {
  // Open a simple file and read the ppm from it
  FILE *f = fopen(filename, "r");
  if (f == NULL) {
    dieerr("Can't open %s for reading\n", filename);
  }
  haloo3d_loadppm(f, tex); // This also calls init so you have to free
  fclose(f);
  printf("Read from %s\n", filename);
}

void load_object(haloo3d_obj *obj, char *filename) {
  // Open a simple file and read the ppm from it
  FILE *f = fopen(filename, "r");
  if (f == NULL) {
    dieerr("Can't open %s for reading\n", filename);
  }
  haloo3d_obj_load(obj, f);
  fclose(f);
  printf("Read from %s\n", filename);
}

void write_framebuffer(haloo3d_fb *fb, char *filename) {
  // And now we should be able to save the framebuffer
  FILE *f = fopen(OUTFILE, "w");
  if (f == NULL) {
    dieerr("Can't open %s for writing\n", OUTFILE);
  }
  haloo3d_writeppm(fb, f);
  fclose(f);
  printf("Wrote to %s\n", OUTFILE);
}

int main(int argc, char **argv) {

  if (argc != 3) {
    dieerr(
        "You must pass in the name of the obj to load and the ppm texture!\n");
  }

  // Load the junk
  haloo3d_obj obj;
  load_object(&obj, argv[1]);
  haloo3d_fb tex;
  load_texture(&tex, argv[2]);

  // Now we create a framebuffer to draw the triangle into
  haloo3d_fb fb;
  haloo3d_fb_init(&fb, WIDTH, HEIGHT);

  haloo3d_facef face;

  // For each face in the model, we draw it with simple orthographic projection
  for (int i = 0; i < ITERATIONS; i++) {
    // REMEMBER TO CLEAR DEPTH BUFFER
    haloo3d_fb_cleardepth(&fb);
    for (int fi = 0; fi < obj.numfaces; fi++) {
      haloo3d_obj_facef(&obj, obj.faces[fi], face);
      // Oh but our zbuffer is actually our w-buffer soooo
      haloo3d_facef_fixw(face);
      // face[0].pos.w = -face[0].pos.z + 1;
      // face[1].pos.w = -face[1].pos.z + 1;
      // face[2].pos.w = -face[2].pos.z + 1;
      // eprintf("orig: (%f,%f) (%f,%f) (%f,%f)\n", face[0].pos.x,
      // face[0].pos.y,
      //         face[1].pos.x, face[2].pos.y, face[2].pos.x, face[2].pos.y);
      // Orthographic projection is literally just draw each point without depth
      haloo3d_viewport_into(face[0].pos.v, WIDTH, HEIGHT);
      haloo3d_viewport_into(face[1].pos.v, WIDTH, HEIGHT);
      haloo3d_viewport_into(face[2].pos.v, WIDTH, HEIGHT);
      if (face[0].pos.z > 0 || face[1].pos.z > 0 || face[2].pos.z > 0) {
        continue;
      }
      // eprintf("screen: (%f,%f) (%f,%f) (%f,%f)\n", face[0].pos.x,
      // face[0].pos.y,
      //         face[1].pos.x, face[2].pos.y, face[2].pos.x, face[2].pos.y);
      haloo3d_texturedtriangle(&fb, &tex, 1.0, face);
    }
  }

  write_framebuffer(&fb, OUTFILE);

  haloo3d_obj_free(&obj);
  haloo3d_fb_free(&tex);
  haloo3d_fb_free(&fb);
}
