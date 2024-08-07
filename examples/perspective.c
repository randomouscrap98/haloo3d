#include "../haloo3d.h"
#include "../haloo3dex_img.h"
#include "../haloo3dex_obj.h"
#include "mathc.h"
#include <stdio.h>

#define WIDTH 640
#define HEIGHT 480
#define FOV 60.0
#define ASPECT ((float)WIDTH / HEIGHT)
#define NEARCLIP 0.01
#define FARCLIP 100.0
#define ITERATIONS 1
#define OUTFILE "perspective.ppm"

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
  FILE *f = fopen(filename, "w");
  if (f == NULL) {
    dieerr("Can't open %s for writing\n", filename);
  }
  haloo3d_writeppm(fb, f);
  fclose(f);
  printf("Wrote to %s\n", filename);
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

  // Create the perspective matrix, which doesn't change
  mfloat_t perspective[MAT4_SIZE];
  mat4_perspective(perspective, to_radians(FOV), ASPECT, NEARCLIP, FARCLIP);

  // Create the camera matrix, which DOES change (or it would if we had user
  // input)
  mfloat_t matrixcam[MAT4_SIZE];
  haloo3d_camera camera;
  haloo3d_camera_init(&camera);
  // Move the camera back
  camera.pos.z += 5.4;

  // Where the final matrix for 3d goes
  mfloat_t matrix3d[MAT4_SIZE];

  // Face storage
  haloo3d_facef face;

  // Where our precalculated vertices go
  struct vec4 *vert_precalc;
  mallocordie(vert_precalc, sizeof(struct vec4) * H3D_OBJ_MAXVERTICES);

  int skipped = 0;

  // For each face in the model, we draw it with simple orthographic projection
  for (int i = 0; i < ITERATIONS; i++) {
    // REMEMBER TO CLEAR DEPTH BUFFER
    haloo3d_fb_cleardepth(&fb);
    // To simulate what would actually happen per frame, let's calc the
    // camera each time
    haloo3d_camera_calclook(&camera, matrixcam);
    // Calculate the actual translation matrix
    mat4_inverse(matrix3d, matrixcam);
    mat4_multiply(matrix3d, matrix3d, perspective);
    // Precalc the vertices from our matrix
    haloo3d_precalc_verts(&obj, matrix3d, vert_precalc);
    for (int fi = 0; fi < obj.numfaces; fi++) {
      // In this program, we're not going to do anything fancy like clipping,
      // we're just going to perspective divide right out the gate
      haloo3d_make_facef(obj.faces[fi], vert_precalc, obj.vtexture, face);
      if (!haloo3d_facef_finalize(face)) {
        // eprintf("SKIPPING TRI: %d\n", fi);
        skipped++;
        continue;
      }
      // haloo3d_facef_fixw(face);
      //  We still have to convert the points into the view
      haloo3d_facef_viewport_into(face, WIDTH, HEIGHT);
      haloo3d_texturedtriangle(&fb, &tex, 1.0, face);
    }
  }

  eprintf("SKIPPED TOTAL: %d FACES: %d VERTS: %d\n", skipped, obj.numfaces,
          obj.numvertices);
  write_framebuffer(&fb, OUTFILE);

  haloo3d_obj_free(&obj);
  haloo3d_fb_free(&tex);
  haloo3d_fb_free(&fb);
}
