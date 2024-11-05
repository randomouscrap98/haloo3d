#include "../haloo3dex_helper.h"
#include "../haloo3dex_obj.h"
#include "../haloo3dex_unigi.h"

// simple afine texture mapped triangle
void triangle(h3d_rastervertex *rv, h3d_fb *buf, h3d_fb *tex, uint16_t bw,
              uint16_t bh) {
  H3DTRI_EASY_BEGIN(rv, bw, bh, linpol, 3, bufi) {
    if (linpol[2] > buf->dbuffer[bufi]) {
      buf->dbuffer[bufi] = linpol[2];
      buf->buffer[bufi] = h3d_fb_getuv(tex, linpol[0], linpol[1]);
    }
    H3DTRI_LINPOL3(linpol);
  }
  H3DTRI_SCAN_END();
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
  h3d_fb_init(&screen, WIDTH, HEIGHT);
  const int len = h3d_fb_size(&screen);
  for (int i = 0; i < len; i++) {
    screen.dbuffer[i] = H3DVF(0);
  }
  eprintf("Initialized screen fb\n");

  h3d_obj model;
  h3d_obj_loadfile(&model, argv[1]);
  eprintf("Object has %d vertices, %d uvs, %d faces\n", model.numvertices,
          model.numvtextures, model.numfaces);

  h3d_fb tex;
  h3d_img_loadppmfile(&tex, argv[2]);
  eprintf("Texture is %dx%d\n", tex.width, tex.height);

  h3d_rastervertex rv[3];

  // Iterate over all triangles
  for (int i = 0; i < model.numfaces; i++) {
    for (int v = 0; v < 3; v++) {
      rv[v].pos[H3DX] =
          WIDTH * (model.vertices[model.faces[i][v].verti][H3DX] + 1) / 2;
      rv[v].pos[H3DY] =
          HEIGHT * (1 - model.vertices[model.faces[i][v].verti][H3DY]) / 2;
      rv[v].interpolants[0] = model.vtexture[model.faces[i][v].texi][H3DX];
      rv[v].interpolants[1] = model.vtexture[model.faces[i][v].texi][H3DY];
      rv[v].interpolants[2] =
          1 / (1 - model.vertices[model.faces[i][v].verti][H3DZ]);
    }
    triangle(rv, &screen, &tex, WIDTH, HEIGHT);
  }
  eprintf("Drew object\n");

  h3d_img_writeppmfile(&screen, OUTFILE);

  h3d_obj_free(&model);
  h3d_fb_free(&tex);
  h3d_fb_free(&screen);
}
