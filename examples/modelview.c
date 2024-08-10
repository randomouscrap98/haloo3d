#include "../haloo3d.h"
#include "../haloo3dex_img.h"
#include "../haloo3dex_obj.h"
#include "mathc.h"
#include <stdio.h>
#include <stdlib.h>

#define DOLIGHTING

// This is expected to run for multiple frames so
// make the default smaller. I don't feel like allowing inputs
#define WIDTH 320
#define HEIGHT 240
#define FOV 60.0
#define ASPECT ((float)WIDTH / HEIGHT)
#define NEARCLIP 0.01
#define FARCLIP 100.0
#define LIGHTANG -MPI / 4.0
#define MINLIGHT 0.5
#define OUTFILE "modelview.ppm"
#define MAXOBJECTS 16

void load_texture(haloo3d_fb *tex, char *filename) {
  // Open a simple file and read the ppm from it
  FILE *f = fopen(filename, "r");
  if (f == NULL) {
    dieerr("Can't open %s for reading\n", filename);
  }
  haloo3d_img_loadppm(f, tex); // This also calls init so you have to free
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
  haloo3d_img_writeppm(fb, f);
  fclose(f);
  printf("Wrote to %s\n", filename);
}

int main(int argc, char **argv) {

  if (argc != 7) {
    dieerr("You must pass in the following:\n- obj file .obj\n- texture file "
           ".ppm\n- obj xofs\n- obj yofs\n- obj zofs\n- obj rotation deg");
  }

  // Load the junk
  haloo3d_obj _obj;
  load_object(&_obj, argv[1]);
  haloo3d_fb _tex;
  load_texture(&_tex, argv[2]);

  // Create the camera matrix, which DOES change
  haloo3d_camera camera;
  haloo3d_camera_init(&camera);

  // Create the perspective matrix, which doesn't change
  mfloat_t perspective[MAT4_SIZE];
  haloo3d_perspective(perspective, FOV, ASPECT, NEARCLIP, FARCLIP);

  // Lighting. Note that for performance, the lighting is always calculated
  // against the base model, and is thus not realistic if the object rotates in
  // the world. This can be fixed easily, since each object gets its own
  // lighting vector, which can easily be rotated in the opposite direction of
  // the model
  struct vec3 light;
  vec3(light.v, 0, -MCOS(LIGHTANG), MSIN(LIGHTANG));

  // We don't have multiple objects but we create an array anyway just in case
  haloo3d_obj_instance objects[MAXOBJECTS];
  haloo3d_objin_init(objects, &_obj, &_tex);
#ifdef DOLIGHTING
  objects[0].lighting = &light;
#endif
  // Move the model based on user input
  objects[0].pos.x = atof(argv[3]);
  objects[0].pos.y = atof(argv[4]);
  objects[0].pos.z = atof(argv[5]);
  mfloat_t yaw = atof(argv[6]);
  vec3(objects[0].lookvec.v, MSIN(yaw), 0, -MCOS(yaw));
  int numobjects = 1;

  // Now we create a framebuffer to draw the triangle into
  haloo3d_fb fb;
  haloo3d_fb_init(&fb, WIDTH, HEIGHT);

  // Storage stuff
  mfloat_t matrix3d[MAT4_SIZE], matrixcam[MAT4_SIZE], matrixscreen[MAT4_SIZE],
      matrixmodel[MAT4_SIZE];
  haloo3d_facef outfaces[H3D_FACEF_MAXCLIP];
  struct vec3 tmp1;
  haloo3d_facef face, baseface;
  struct vec4 *vert_precalc;
  mallocordie(vert_precalc, sizeof(struct vec4) * H3D_OBJ_MAXVERTICES);

  // -----------------------------------
  //     Actual rendering
  // -----------------------------------

  // REMEMBER TO CLEAR DEPTH BUFFER
  haloo3d_fb_cleardepth(&fb);

  // Screen matrix calc. We multiply the modelview matrix with this later
  haloo3d_camera_calclook(&camera, matrixcam);
  mat4_inverse(matrixcam, matrixcam);
  mat4_multiply(matrixscreen, perspective, matrixcam);

  // Iterate over objects
  for (int i = 0; i < numobjects; i++) {
    // Setup final model matrix and the precalced vertices
    vec3_add(tmp1.v, objects[i].pos.v, objects[i].lookvec.v);
    haloo3d_my_lookat(matrixmodel, objects[i].pos.v, tmp1.v, camera.up.v);
    mat4_multiply_f(matrixmodel, matrixmodel, objects[i].scale);
    mat4_multiply(matrix3d, matrixscreen, matrixmodel);
    haloo3d_precalc_verts(objects[i].model, matrix3d, vert_precalc);
    // Iterate over object faces
    for (int fi = 0; fi < objects[i].model->numfaces; fi++) {
      // Copy face values out of precalc array and clip them
      haloo3d_make_facef(objects[i].model->faces[fi], vert_precalc,
                         objects[i].model->vtexture, face);
      int tris = haloo3d_facef_clip(face, outfaces);
      for (int ti = 0; ti < tris; ti++) {
        // You (perhaps unfortunately) still need to finalize the face. This
        // lets you ignore backface culling if you want (we turn it on here)
        if (!haloo3d_facef_finalize(outfaces[tris])) {
          continue;
        }
        mfloat_t intensity = 1.0;
        if (objects[i].lighting) {
          haloo3d_obj_facef(objects[i].model, objects[i].model->faces[fi],
                            baseface);
          intensity =
              haloo3d_calc_light(objects[i].lighting->v, MINLIGHT, baseface);
        }
        //   We still have to convert the points into the view
        haloo3d_facef_viewport_into(outfaces[ti], WIDTH, HEIGHT);
        haloo3d_texturedtriangle(&fb, objects[i].texture, intensity,
                                 outfaces[ti]);
      }
    }
  }

  write_framebuffer(&fb, OUTFILE);

  haloo3d_obj_free(&_obj);
  haloo3d_fb_free(&_tex);
  haloo3d_fb_free(&fb);
}
