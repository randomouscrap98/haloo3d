#include "haloo3dex_gen.h"
#include "haloo3d.h"

void haloo3d_gen_checkerboard(haloo3d_fb *fb, uint16_t *cols, uint16_t numcols,
                              uint16_t size) {
  haloo3d_fb_init_tex(fb, size, size);
  for (int y = 0; y < fb->height; y++) {
    for (int x = 0; x < fb->width; x++) {
      haloo3d_fb_set(fb, x, y, cols[(x + y) % numcols]);
    }
  }
}

void haloo3d_gen_1pxgradient(haloo3d_fb *fb, uint16_t bottom, uint16_t top,
                             uint16_t height) {
  haloo3d_fb_init_tex(fb, 1, height);
  for (int y = 0; y < fb->height; y++) {
    haloo3d_fb_set(
        fb, 0, y,
        haloo3d_col_lerp(top, bottom, (mfloat_t)y / (fb->height - 1)));
  }
}

// This technically inits the obj, so you will need to free it
void haloo3d_gen_skybox(haloo3d_obj *obj) {
  mallocordie(obj->vertices, 8 * sizeof(struct vec4));
  mallocordie(obj->vtexture, 4 * sizeof(struct vec3));
  mallocordie(obj->faces, 12 * sizeof(haloo3d_facei));
  obj->numvertices = 8;
  obj->numvtextures = 4;
  obj->numfaces = 12;
  // There are only four corners of a texture and we're making a box... yeah
  vec3(obj->vtexture[0].v, 0.001, 0.001, 0); // Oops, it sometimes overflows
  vec3(obj->vtexture[1].v, 0.001, 0.999, 0);
  vec3(obj->vtexture[2].v, 0.999, 0.001, 0); // Oops, it sometimes overflows
  vec3(obj->vtexture[3].v, 0.999, 0.999, 0);
  // Cube faces are weird, I guess just manually do them? ugh
  // First 4 are the bottom vertices. We can make two faces out of these
  vec3(obj->vertices[0].v, -1, -1, -1);
  vec3(obj->vertices[1].v, 1, -1, -1);
  vec3(obj->vertices[2].v, 1, -1, 1);
  vec3(obj->vertices[3].v, -1, -1, 1);
  // Now the top 4 vertices, same order as bottom
  vec3(obj->vertices[4].v, -1, 1, -1);
  vec3(obj->vertices[5].v, 1, 1, -1);
  vec3(obj->vertices[6].v, 1, 1, 1);
  vec3(obj->vertices[7].v, -1, 1, 1);
  // clang-format off
  // The face vertices
  const uint8_t fv[12][3] = {
		{0, 2, 1}, // bottom
		{0, 3, 2},
		{4, 5, 6}, // top
		{6, 7, 4},
		{0, 1, 5}, // south
		{5, 4, 0},
		{1, 2, 6}, // east
		{6, 5, 1},
		{2, 3, 7}, // North
		{7, 6, 2},
		{3, 0, 4}, // west
		{4, 7, 3},
  };
  // Vertex texture index for each face corner
  const uint8_t vt[12][3] = {
		{0, 2, 0}, // bottom
		{2, 0, 0},
		{1, 3, 1}, // top
		{1, 3, 3},
		{0, 2, 3}, // south
		{3, 1, 0},
		{0, 2, 3}, // east
		{3, 1, 0},
		{0, 2, 3}, // North
		{3, 1, 0},
		{0, 2, 3}, // west
		{3, 1, 0},
  };
  // clang-format on
  for (int fi = 0; fi < obj->numfaces; fi++) {
    for (int v = 0; v < 3; v++) {
      obj->faces[fi][v].posi = fv[fi][v];
      obj->faces[fi][v].texi = vt[fi][v];
    }
  }
}
