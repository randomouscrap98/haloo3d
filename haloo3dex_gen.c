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

void haloo3d_gen_boxvtexture(struct vec3 *textures) {
  vec3(textures[0].v, 0.001, 0.001, 0); // Oops, it sometimes overflows
  vec3(textures[1].v, 0.001, 0.999, 0);
  vec3(textures[2].v, 0.999, 0.001, 0); // Oops, it sometimes overflows
  vec3(textures[3].v, 0.999, 0.999, 0);
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
  haloo3d_gen_boxvtexture(obj->vtexture);
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

void haloo3d_gen_plane(haloo3d_obj *obj, uint16_t size) {
  obj->numvertices = (size + 1) * (size + 1);
  obj->numvtextures = 4;
  obj->numfaces = 2 * size * size;
  mallocordie(obj->vertices, obj->numvertices * sizeof(struct vec4));
  mallocordie(obj->vtexture, obj->numvtextures * sizeof(struct vec3));
  mallocordie(obj->faces, obj->numfaces * sizeof(haloo3d_facei));
  // Vtexture is just the four corners again
  haloo3d_gen_boxvtexture(obj->vtexture);
  // Generate all the simple vertices along the plane at y=0
  int i = 0;
  for (mfloat_t z = -size / 2.0; z <= size / 2.0; z += 1) {
    for (mfloat_t x = -size / 2.0; x <= size / 2.0; x += 1) {
      vec3(obj->vertices[i].v, x, 0, z);
      i++;
    }
  }
  i = 0;
  // Faces are slightly different; we generate two for every "cell" inside the
  // vertices
  for (int z = 0; z < size; z++) {
    for (int x = 0; x < size; x++) {
      int topleft = x + z * (size + 1);
      int topright = x + 1 + z * (size + 1);
      int bottomleft = x + (z + 1) * (size + 1);
      int bottomright = x + 1 + (z + 1) * (size + 1);
      // remember to wind counter-clockwise
      obj->faces[i][0].posi = topleft;
      obj->faces[i][0].texi = 1;
      obj->faces[i][1].posi = bottomleft;
      obj->faces[i][1].texi = 0;
      obj->faces[i][2].posi = topright;
      obj->faces[i][2].texi = 3;
      i++;
      obj->faces[i][0].posi = topright;
      obj->faces[i][0].texi = 3;
      obj->faces[i][1].posi = bottomleft;
      obj->faces[i][1].texi = 0;
      obj->faces[i][2].posi = bottomright;
      obj->faces[i][2].texi = 2;
      i++;
      // obj.Faces = append(obj.Faces, hrend.Facei{
      // 	{Posi: topleft, Texi: 0},
      // 	{Posi: bottomleft, Texi: 2},
      // 	{Posi: topright, Texi: 1},
      // }, hrend.Facei{
      // 	{Posi: topright, Texi: 1},
      // 	{Posi: bottomleft, Texi: 2},
      // 	{Posi: bottomright, Texi: 3},
      // })
    }
  }
  eprintf("Generated plane with %d vertices, %d faces\n", obj->numvertices,
          obj->numfaces);
}

// <BS>#define _H3D_GEN_SLMOD(x, y, xmod, ymod) {
// void haloo3d_gen_sloped_modpoint(uint16_t x, uint16_t y, int xmod, int ymod,
// mfloat_t slopiness, mfloat_t downbias) {
//   //First, get the point to inspect
//
// }

void haloo3d_gen_sloped(haloo3d_obj *obj, uint16_t size, mfloat_t slopiness,
                        mfloat_t downbias) {
  haloo3d_gen_plane(obj, size);
  size++; // Actual vertex size is +1
  uint16_t center = size >> 1;
  // This is one less the length of one edge of the square as we expand outward
  for (int i = 2; i <= size; i += 2) {
    uint16_t ofs = (i >> 1);
    // Just go in a square around the "center" point, modifying each point such
    // that it is some movement away from the one adjacent to it towards the
    // center
    for (int j = 0; j < i; j++) {
      uint16_t pi, ti;
      for (int c = 0; c < 4; c++) {
        uint16_t search = -(rand() % (3 - (j < 2) - (j < 1))) + 1;
        switch (c) {
        case 0: // Top left->right
          pi = center - ofs + j + size * (center - ofs);
          ti = pi + size + search;
          break;
        case 1: // Top right->down
          pi = center + ofs + size * (center - ofs + j);
          ti = pi + (search * size) - 1;
          break;
        case 2: // Bottom right->left
          pi = center + ofs - j + size * (center + ofs);
          ti = pi - size - search;
          break;
        case 3: // Bottom left->up
          pi = center - ofs + size * (center + ofs - j);
          ti = pi - (search * size) + 1;
          break;
        }
        if (pi < 0 || ti < 0 || pi >= obj->numvertices ||
            ti >= obj->numvertices) {
          continue;
        }
        obj->vertices[pi].y =
            obj->vertices[ti].y +
            slopiness * ((mfloat_t)rand() / RAND_MAX - downbias);
      }
    }
  }
}
