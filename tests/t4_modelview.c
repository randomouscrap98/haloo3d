#include "../haloo3d.h"
#include "../haloo3d_3d.h"
#include "../haloo3d_helper.h"
#include "../haloo3d_obj.h"
#include "../haloo3d_special.h"
#include "../haloo3d_unigi.h"

// #include <stdio.h>
#include <stdlib.h>
#include <string.h>

// This is expected to run for multiple frames so
// make the default smaller. I don't feel like allowing inputs
#define WIDTH 1280
#define HEIGHT 720
#define FOV 60.0
#define ASPECT ((float)WIDTH / HEIGHT)
#define NEARCLIP 0.01
#define FARCLIP 100.0
#define LIGHTANG -MPI / 4.0
#define MINLIGHT 0.5
#define OUTFILE "t4_modelview.ppm"
#define NUMINTERPOLANTS 3

// simple perspective-correct texture mapped triangle with depth buffer
void triangle(h3d_rastervert *rv, h3d_fb *buf, h3d_fb *tex) {
  H3DTRI_EASY_BEGIN(rv, buf->width, buf->height, linpol, 3, bufi) {
    if (linpol[2] > buf->dbuffer[bufi]) {
      buf->dbuffer[bufi] = linpol[2];
      // 1/z is linear across triangle, need z for uv
      float_t z = 1 / linpol[2];
      buf->buffer[bufi] = h3d_fb_getuv(tex, linpol[0] * z, linpol[1] * z);
    }
    H3DTRI_LINPOL3(linpol);
  }
  H3DTRI_SCAN_END(buf->width);
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

  // Create a framebuffer to draw the triangle into
  h3d_fb fb;
  h3d_fb_init(&fb, WIDTH, HEIGHT);

  // Create the perspective matrix, which doesn't change
  mat4 perspective;
  h3d_perspective(FOV, ASPECT, NEARCLIP, FARCLIP, perspective);

  // Figure out model params for translation
  vec3 pos;
  VEC3(pos, atof(argv[3]), atof(argv[4]), atof(argv[5]));
  vec3 lookvec;
  float_t yaw = atof(argv[6]);
  VEC3(lookvec, sinf(yaw), 0, -cosf(yaw));
  vec3 up;
  VEC3(up, 0, 1, 0);
  vec3 scale;
  VEC3(scale, 1, 1, 1);

  // -----------------------------------
  //     Actual rendering
  // -----------------------------------

  // REMEMBER TO CLEAR DEPTH BUFFER
  const int len = h3d_fb_size(&fb);
  for (int i = 0; i < len; i++) {
    fb.buffer[i] = 0xF0F0;
    fb.dbuffer[i] = H3DVF(0);
  }

  // NOTE: we do not change the camera in this one, only the model!
  mat4 modelmatrix;
  h3d_model_matrix(pos, lookvec, up, scale, modelmatrix);
  mat4 finalmatrix;
  mat4_multiply(perspective, modelmatrix, finalmatrix);

  vec4 verttrans[H3D_OBJ_MAXVERTICES];
  h3d_obj_batchtranslate(&_obj, finalmatrix, verttrans);

  h3d_3dface clipfaces[H3D_FACEF_MAXCLIP];

  // Iterate over object faces
  for (int fi = 0; fi < _obj.numfaces; fi++) {
    h3d_3dface face;
    for (int v = 0; v < 3; v++) {
      memcpy(face[v].pos, verttrans[_obj.faces[fi][v].verti], sizeof(vec4));
      // For our perspective-correct textures, our interpolants are u/z, v/z,
      // and 1/z
      float_t invz = 1 / face[v].pos[H3DW];
      face[v].interpolants[0] =
          _obj.vtexture[_obj.faces[fi][v].texi][H3DX] * invz;
      face[v].interpolants[1] =
          _obj.vtexture[_obj.faces[fi][v].texi][H3DY] * invz;
      face[v].interpolants[2] = invz;
    }

    int tris = h3d_3dface_clip(face, clipfaces, NUMINTERPOLANTS);
    h3d_rasterface rface;
    for (int ti = 0; ti < tris; ti++) {
      // You (perhaps unfortunately) still need to finalize the face. This
      // lets you ignore backface culling if you want (we turn it on here)
      if (!h3d_3dface_normalize(clipfaces[ti])) {
        continue;
      }
      for (int v = 0; v < 3; v++) {
        memcpy(rface[v].interpolants, clipfaces[ti][v].interpolants,
               sizeof(float_t) * NUMINTERPOLANTS);
        h3d_viewport(clipfaces[ti][v].pos, WIDTH, HEIGHT, rface[v].pos);
      }
      triangle(rface, &fb, &_tex);
    }
  }

  h3d_img_writeppmfile(&fb, OUTFILE);

  h3d_obj_free(&_obj);
  h3d_fb_free(&_tex);
  h3d_fb_free(&fb);
}
