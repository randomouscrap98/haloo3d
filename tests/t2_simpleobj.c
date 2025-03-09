#include "../haloo3d_ex.h"
#include "../haloo3d_obj.h"

#include <math.h>

// simple affine texture mapped triangle with depth buffer
void triangle(h3d_rastervert *rv, h3d_fb *buf, h3d_fb *tex) {
  H3DTRI_EASY_BEGIN(rv, buf->width, buf->height, linpol, 3, bufi) {
    if (linpol[2] < buf->dbuffer[bufi]) {
      buf->dbuffer[bufi] = linpol[2];
      buf->buffer[bufi] = H3D_FB_GETUV(tex, linpol[0], linpol[1]);
    }
    H3DTRI_LINPOL3(linpol);
  }
  H3DTRI_SCAN_END(buf->width);
}

#define WIDTH 320
#define HEIGHT 320
#define OUTFILE "t2_simpleobj.ppm"

int main(int argc, char **argv) {
  eprintf("Program start\n");
  if (argc != 3) {
    dieerr("Must pass in an obj model file and ppm texture file!\n");
  }

  h3d_fb screen;
  H3D_FB_INIT(&screen, WIDTH, HEIGHT, sizeof(uint16_t));
  const int len = H3D_FB_SIZE(&screen);
  for (int i = 0; i < len; i++) {
    screen.buffer[i] = H3DC_A1R5G5B5_F(1.0, 0, 1.0, 0);
    screen.dbuffer[i] = H3DVF(999999);
  }
  eprintf("Initialized screen fb\n");

  h3d_obj model;
  h3d_obj_loadfile(&model, argv[1]);
  eprintf("Object has %d vertices, %d uvs, %d faces\n", model.numvertices,
          model.numvtextures, model.numfaces);

  h3d_fb tex;
  h3d_fb_loadppmfile(&tex, argv[2], h3d_fb_in_A1R5G5B5);
  eprintf("Texture is %dx%d\n", tex.width, tex.height);

  h3d_rasterface rv;

  // Iterate over all triangles
  for (int i = 0; i < model.numfaces; i++) {
    for (int v = 0; v < 3; v++) {
      rv[v].pos[H3DX] = round(
          WIDTH * 0.5 * (model.vertices[model.faces[i][v].verti][H3DX] + 1));
      rv[v].pos[H3DY] = round(
          HEIGHT * 0.5 * (1 - model.vertices[model.faces[i][v].verti][H3DY]));
      rv[v].interpolants[0] = model.vtexture[model.faces[i][v].texi][H3DX];
      rv[v].interpolants[1] = model.vtexture[model.faces[i][v].texi][H3DY];
      rv[v].interpolants[2] =
          1 / (3 + model.vertices[model.faces[i][v].verti][H3DZ]);
      // 3 is just an arbitrary number to make sure z is never 0
    }
    triangle(rv, &screen, &tex);
  }
  eprintf("Drew object\n");

  h3d_fb_writeppmfile(&screen, OUTFILE, h3d_fb_out_A1R5G5B5);

  h3d_obj_free(&model);
  H3D_FB_FREE(&tex);
  H3D_FB_FREE(&screen);
}
