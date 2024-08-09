#include "../haloo3d.h"
#include "../haloo3dex_gen.h"
#include "../haloo3dex_img.h"
#include "../haloo3dex_obj.h"
#include "../haloo3dex_print.h"
#include "mathc.h"
#include <stdlib.h>
#include <time.h>

#define DOLIGHTING

#define WIDTH 640
#define HEIGHT 480
#define ASPECT ((float)WIDTH / HEIGHT)
#define FOV 60.0
#define NEARCLIP 0.01
#define FARCLIP 100.0
#define LIGHTANG -MPI / 4.0
#define MINLIGHT 0.25
#define OUTFILE "scene.ppm"

#define NUMOBJECTS 2

int main(int argc, char **argv) {

  if (argc != 7) {
    dieerr("You must pass in the following:\n- obj file .obj\n- texture file "
           ".ppm\n- obj xofs\n- obj yofs\n- obj zofs\n- obj rotation deg\n");
  }

  // Load the junk + generate stuff
  haloo3d_obj models[NUMOBJECTS];
  haloo3d_fb textures[NUMOBJECTS];
  haloo3d_obj_loadfile(models, argv[1]);
  haloo3d_img_loadppmfile(textures, argv[2]);
  haloo3d_gen_1pxgradient(textures + 1, 0xF44F, 0xF004, 32);
  haloo3d_gen_skybox(models + 1);

  // Create the camera matrix, which DOES change. In this one,
  // we move the camera instead of the model
  haloo3d_camera camera;
  haloo3d_camera_init(&camera);
  camera.pos.x = atof(argv[3]);
  camera.pos.y = atof(argv[4]);
  camera.pos.z = atof(argv[5]);
  camera.yaw = atof(argv[6]);

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

  haloo3d_obj_instance objects[NUMOBJECTS];
  for (int i = 0; i < NUMOBJECTS; i++) {
    haloo3d_objin_init(objects + i, models + i, textures + i);
  }
#ifdef DOLIGHTING
  objects[0].lighting = &light;
#endif
  objects[0].pos.z = -1.5;
  objects[1].scale = 50;

  // Now we create a framebuffer to draw the triangle into
  haloo3d_fb fb;
  haloo3d_fb_init(&fb, WIDTH, HEIGHT);

  // Printing to screen needs tracking
  haloo3d_print_tracker t;
  char printbuf[8192];
  haloo3d_print_initdefault(&t, printbuf, sizeof(printbuf));
  t.fb = &fb;

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

  clock_t begin = clock();

  // REMEMBER TO CLEAR DEPTH BUFFER
  haloo3d_fb_cleardepth(&fb);

  // Screen matrix calc. We multiply the modelview matrix with this later
  haloo3d_camera_calclook(&camera, matrixcam);
  mat4_inverse(matrixcam, matrixcam);
  mat4_multiply(matrixscreen, perspective, matrixcam);

  // Iterate over objects
  for (int i = 0; i < NUMOBJECTS; i++) {
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

  clock_t end = clock();
  haloo3d_print(&t, "Frame time: %.2f",
                1000.0 * (float)(end - begin) / CLOCKS_PER_SEC);

  haloo3d_img_writeppmfile(&fb, OUTFILE);

  for (int i = 0; i < NUMOBJECTS; i++) {
    haloo3d_obj_free(models + i);
    haloo3d_fb_free(textures + i);
  }
}
