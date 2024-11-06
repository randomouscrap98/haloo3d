#include "../haloo3d.h"
#include "../haloo3d_3d.h"
#include "../haloo3d_helper.h"
#include "../haloo3d_obj.h"
#include "../haloo3d_unigi.h"

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
#define OUTFILE "t4_modelview.ppm"

// simple affine texture mapped triangle with depth buffer
void triangle(h3d_rastervertex *rv, h3d_fb *buf, h3d_fb *tex, uint16_t bw,
              uint16_t bh) {
  H3DTRI_EASY_BEGIN(rv, bw, bh, linpol, 3, bufi) {
    if (linpol[2] < buf->dbuffer[bufi]) {
      buf->dbuffer[bufi] = linpol[2];
      buf->buffer[bufi] = h3d_fb_getuv(tex, linpol[0], linpol[1]);
    }
    H3DTRI_LINPOL3(linpol);
  }
  H3DTRI_SCAN_END();
}

int main(int argc, char **argv) {

  if (argc != 7) {
    dieerr("You must pass in the following:\n- obj file .obj\n- texture file "
           ".ppm\n- obj xofs\n- obj yofs\n- obj zofs\n- obj rotation deg");
  }

  // Load the junk
  h3d_obj _obj;
  h3d_obj_loadfile(&_obj, argv[1]);
  h3d_fb _tex;
  h3d_img_loadppmfile(&_tex, argv[2]);

  // Create the camera matrix, which DOES change
  h3d_camera camera;
  h3d_camera_init(&camera);

  // Create the perspective matrix, which doesn't change
  mat4 perspective;
  h3d_perspective(FOV, ASPECT, NEARCLIP, FARCLIP, perspective);

  // Lighting. Note that for performance, the lighting is always calculated
  // against the base model, and is thus not realistic if the object rotates in
  // the world. This can be fixed easily, since each object gets its own
  // lighting vector, which can easily be rotated in the opposite direction of
  // the model
  vec3 light;
  VEC3(light, 0, -cosf(LIGHTANG), sinf(LIGHTANG));

  // #ifdef DOLIGHTING
  //   objects[0].lighting = &light;
  // #endif

  // Move the model based on user input
  vec3 pos;
  VEC3(pos, atof(argv[3]), atof(argv[4]), atof(argv[5]));

  vec3 lookvec;
  float_t yaw = atof(argv[6]);
  VEC3(lookvec, sinf(yaw), 0, -cosf(yaw));

  // Now we create a framebuffer to draw the triangle into
  h3d_fb fb;
  h3d_fb_init(&fb, WIDTH, HEIGHT);

  // Storage stuff
  mat4 matrix3d, matrixcam, matrixscreen, matrixmodel;
  // haloo3d_facef outfaces[H3D_FACEF_MAXCLIP];
  vec3 tmp1;
  // haloo3d_facef face, baseface;
  //  vec4 *vert_precalc;
  //  mallocordie(vert_precalc, sizeof(struct vec4) * H3D_OBJ_MAXVERTICES);

  // -----------------------------------
  //     Actual rendering
  // -----------------------------------

  // REMEMBER TO CLEAR DEPTH BUFFER
  const int len = h3d_fb_size(&fb);
  for (int i = 0; i < len; i++) {
    fb.buffer[i] = 0xF0F0;
    fb.dbuffer[i] = H3DVF(999999);
  }

  // Screen matrix calc. We multiply the modelview matrix with this later
  h3d_camera_calclook(&camera, matrixcam, NULL);
  mat4_inverse(matrixcam, matrixcam);
  mat4_multiply(perspective, matrixcam, matrixscreen);

  // Setup final model matrix and the precalced vertices
  vec3_add(pos, lookvec, tmp1);
  h3d_my_lookat(pos, tmp1, camera.up, matrixmodel);
  // mat4_multiply_f(matrixmodel, matrixmodel, objects[i].scale);
  mat4_multiply(matrix3d, matrixscreen, matrixmodel);
  haloo3d_precalc_verts(objects[i].model, matrix3d, vert_precalc);
  // Iterate over object faces
  for (int fi = 0; fi < _obj.numfaces; fi++) {
    // Copy face values out of precalc array and clip them
    haloo3d_make_facef(objects[i].model->faces[fi], vert_precalc,
                       objects[i].model->vtexture, face);
    int tris = haloo3d_facef_clip(face, outfaces);
    rsettings.texture = objects[i].texture;
    for (int ti = 0; ti < tris; ti++) {
      // You (perhaps unfortunately) still need to finalize the face. This
      // lets you ignore backface culling if you want (we turn it on here)
      if (!haloo3d_facef_finalize(outfaces[tris])) {
        continue;
      }
      // rsettings.intensity = 1.0;
      // if (objects[i].lighting) {
      //   haloo3d_obj_facef(objects[i].model, objects[i].model->faces[fi],
      //                     baseface);
      //   rsettings.intensity =
      //       haloo3d_calc_light(objects[i].lighting->v, MINLIGHT, baseface);
      // }
      //   We still have to convert the points into the view
      haloo3d_facef_viewport_into(outfaces[ti], WIDTH, HEIGHT);
      haloo3d_triangle(&fb, &rsettings, outfaces[ti]);
    }
  }

  write_framebuffer(&fb, OUTFILE);

  haloo3d_obj_free(&_obj);
  haloo3d_fb_free(&_tex);
  haloo3d_fb_free(&fb);
}
