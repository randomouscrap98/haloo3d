#include "haloo3dex_gen.h"
#include "haloo3d.h"
#include "lib/FastNoiseLite.h"

// how do i wnat to generte the layers?
// - Alpha? some parts are blank?
// - Noise? Hmmm
// - Shapes?
// Imagine generating the following:
// - Dirt: just a brown layer with noise applied directly to it
// - Checkerboard: alternating colors as already implemented, but with...
//   the ability to merge alpha? yeah
// - What if checkerboard dark? Just black checkerboard with transparency
// - Bricks: red base, add small noise, brick lines, add small noise again
// - Now that we know, textures should always be NOT generated, so we can
//   apply them on top of each other. Or could we check to see if it's init?
//   no, values are all random.
// - How do we stack transaprent values?

void haloo3d_apply_alternating(haloo3d_fb *fb, uint16_t *cols,
                               uint16_t numcols) {
  for (int y = 0; y < fb->height; y++) {
    for (int x = 0; x < fb->width; x++) {
      uint16_t basecol = haloo3d_fb_get(fb, x, y);
      haloo3d_fb_set(fb, x, y,
                     haloo3d_col_blend(cols[(x + y) % numcols], basecol));
    }
  }
}

void haloo3d_apply_vgradient(haloo3d_fb *fb, uint16_t top, uint16_t bottom) {
  for (int y = 0; y < fb->height; y++) {
    uint16_t col =
        haloo3d_col_lerp(top, bottom, (mfloat_t)y / (fb->height - 1));
    for (int x = 0; x < fb->width; x++) {
      uint16_t basecol = haloo3d_fb_get(fb, x, y);
      haloo3d_fb_set(fb, x, y, haloo3d_col_blend(col, basecol));
    }
  }
}

void haloo3d_apply_noise(haloo3d_fb *fb, float *noise, float scale) {
  static int noisenext = 0;
  int malloced = 0;
  if (noise == NULL) {
    mallocordie(noise, sizeof(float) * fb->width * fb->height);
    fnl_state ns = fnlCreateState();
    ns.noise_type = FNL_NOISE_OPENSIMPLEX2;
    ns.seed = noisenext++;
    ns.frequency = 1;
    for (int y = 0; y < fb->height; y++) {
      for (int x = 0; x < fb->width; x++) {
        noise[x + y * fb->width] = fnlGetNoise2D(&ns, x, y);
      }
    }
  }
  uint16_t noisealpha = scale * 15;
  for (int y = 0; y < fb->height; y++) {
    for (int x = 0; x < fb->width; x++) {
      uint16_t basecol = haloo3d_fb_get(fb, x, y);
      uint16_t noisecolv = (noise[x + y * fb->width] + 1.0) / 2.0 * 15;
      uint16_t noisecol =
          H3DC_ARGB(noisealpha, noisecolv, noisecolv, noisecolv);
      haloo3d_fb_set(fb, x, y, haloo3d_col_blend(noisecol, basecol));
    }
  }
  if (malloced) {
    free(noise);
  }
}

void haloo3d_apply_brick(haloo3d_fb *fb, uint16_t width, uint16_t height,
                         uint16_t color) {
  int i = 0;
  for (int y = height - 1; y < fb->height + height; y += height) {
    int yofs = (width * (2 + 5 * (i & 1))) / 10;
    for (int x = 0; x < fb->width; x++) {
      if (y < fb->height) {
        uint16_t basecol = haloo3d_fb_get(fb, x, y);
        haloo3d_fb_set(fb, x, y, haloo3d_col_blend(color, basecol));
      }
      if (x == yofs) {
        for (int v = y - (height - 1); v < y; v++) {
          if (v < fb->height) {
            uint16_t basecol = haloo3d_fb_get(fb, x, v);
            haloo3d_fb_set(fb, x, v, haloo3d_col_blend(color, basecol));
          }
        }
        yofs += width;
      }
    }
    i++;
  }
}

void haloo3d_gen_obj_prealloc(haloo3d_obj *obj, uint16_t numverts,
                              uint16_t numvtex, uint16_t numfaces) {
  obj->numvertices = numverts;
  obj->numvtextures = numvtex;
  obj->numfaces = numfaces;
  mallocordie(obj->vertices, obj->numvertices * sizeof(struct vec4));
  mallocordie(obj->vtexture, obj->numvtextures * sizeof(struct vec3));
  mallocordie(obj->faces, obj->numfaces * sizeof(haloo3d_facei));
}

void haloo3d_gen_boxvtexture(struct vec3 *textures) {
  vec3(textures[0].v, 0.001, 0.001, 0); // Oops, it sometimes overflows
  vec3(textures[1].v, 0.001, 0.999, 0);
  vec3(textures[2].v, 0.999, 0.001, 0); // Oops, it sometimes overflows
  vec3(textures[3].v, 0.999, 0.999, 0);
}

// This technically inits the obj, so you will need to free it
void haloo3d_gen_skybox(haloo3d_obj *obj) {
  haloo3d_gen_obj_prealloc(obj, 8, 4, 12);
  // There are only four corners of a texture and we're making a box... yeah
  haloo3d_gen_boxvtexture(obj->vtexture);
  // Cube faces are weird, I guess just manually do them? ugh
  // First 4 are the bottom vertices. We can make two faces out of these
  vec4(obj->vertices[0].v, -1, -1, -1, 1);
  vec4(obj->vertices[1].v, 1, -1, -1, 1);
  vec4(obj->vertices[2].v, 1, -1, 1, 1);
  vec4(obj->vertices[3].v, -1, -1, 1, 1);
  // Now the top 4 vertices, same order as bottom
  vec4(obj->vertices[4].v, -1, 1, -1, 1);
  vec4(obj->vertices[5].v, 1, 1, -1, 1);
  vec4(obj->vertices[6].v, 1, 1, 1, 1);
  vec4(obj->vertices[7].v, -1, 1, 1, 1);
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
  haloo3d_gen_obj_prealloc(obj, (size + 1) * (size + 1), 4, 2 * size * size);
  // Vtexture is just the four corners again
  haloo3d_gen_boxvtexture(obj->vtexture);
  // Generate all the simple vertices along the plane at y=0
  int i = 0;
  for (mfloat_t z = -size / 2.0; z <= size / 2.0; z += 1) {
    for (mfloat_t x = -size / 2.0; x <= size / 2.0; x += 1) {
      vec4(obj->vertices[i].v, x, 0, z, 1);
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
    }
  }
  eprintf("Generated plane with %d vertices, %d faces\n", obj->numvertices,
          obj->numfaces);
}

// Generate a basic grid of given size. Expect walls to be viewed from both
// sides
void haloo3d_gen_grid(haloo3d_obj *obj, uint16_t size) {
  haloo3d_gen_obj_prealloc(obj, 2 * (size + 1) * (size + 1), 4,
                           4 * size * (size + 1));
  // Vtexture is just the four corners again
  haloo3d_gen_boxvtexture(obj->vtexture);
  // Generate the bottom vertices. It's the same as the plane
  int i = 0;
  for (mfloat_t z = -size / 2.0; z <= size / 2.0; z += 1) {
    for (mfloat_t x = -size / 2.0; x <= size / 2.0; x += 1) {
      vec4(obj->vertices[i].v, x, 0, z, 1);
      i++;
    }
  }
  // Generate the top vertices. It's the same as the plane
  for (mfloat_t z = -size / 2.0; z <= size / 2.0; z += 1) {
    for (mfloat_t x = -size / 2.0; x <= size / 2.0; x += 1) {
      vec4(obj->vertices[i].v, x, 1, z, 1);
      i++;
    }
  }
  i = 0;
  int vertplanesize = (size + 1) * (size + 1);
  // Generate the normal faces on the top left corners
  for (int z = 0; z < size; z++) {
    for (int x = 0; x < size; x++) {
      int topleft = x + z * (size + 1);
      int topright = x + 1 + z * (size + 1);
      int bottomleft = x + (z + 1) * (size + 1);
      int topleft2 = topleft + vertplanesize;
      int topright2 = topright + vertplanesize;
      int bottomleft2 = bottomleft + vertplanesize;
      // remember to wind counter-clockwise
      obj->faces[i][0].posi = topleft;
      obj->faces[i][0].texi = 0;
      obj->faces[i][1].posi = topright;
      obj->faces[i][1].texi = 2;
      obj->faces[i][2].posi = topleft2;
      obj->faces[i][2].texi = 1;
      i++;
      obj->faces[i][0].posi = topleft2;
      obj->faces[i][0].texi = 1;
      obj->faces[i][1].posi = topright;
      obj->faces[i][1].texi = 2;
      obj->faces[i][2].posi = topright2;
      obj->faces[i][2].texi = 3;
      i++;
      obj->faces[i][0].posi = bottomleft;
      obj->faces[i][0].texi = 0;
      obj->faces[i][1].posi = topleft;
      obj->faces[i][1].texi = 2;
      obj->faces[i][2].posi = bottomleft2;
      obj->faces[i][2].texi = 1;
      i++;
      obj->faces[i][0].posi = bottomleft2;
      obj->faces[i][0].texi = 1;
      obj->faces[i][1].posi = topleft;
      obj->faces[i][1].texi = 2;
      obj->faces[i][2].posi = topleft2;
      obj->faces[i][2].texi = 3;
      i++;
    }
  }
  // And then we generate some walls along the edges we didn't finish
  for (int z = 0; z < size; z++) {
    int topright = size + z * (size + 1);
    int bottomright = size + (z + 1) * (size + 1);
    int topright2 = topright + vertplanesize;
    int bottomright2 = bottomright + vertplanesize;
    // remember to wind counter-clockwise
    obj->faces[i][0].posi = topright;
    obj->faces[i][0].texi = 0;
    obj->faces[i][1].posi = bottomright;
    obj->faces[i][1].texi = 2;
    obj->faces[i][2].posi = topright2;
    obj->faces[i][2].texi = 1;
    i++;
    obj->faces[i][0].posi = topright2;
    obj->faces[i][0].texi = 1;
    obj->faces[i][1].posi = bottomright;
    obj->faces[i][1].texi = 2;
    obj->faces[i][2].posi = bottomright2;
    obj->faces[i][2].texi = 3;
    i++;
  }
  for (int x = 0; x < size; x++) {
    int bottomleft = x + size * (size + 1);
    int bottomright = x + 1 + size * (size + 1);
    int bottomleft2 = bottomleft + vertplanesize;
    int bottomright2 = bottomright + vertplanesize;
    // remember to wind counter-clockwise
    obj->faces[i][0].posi = bottomleft;
    obj->faces[i][0].texi = 0;
    obj->faces[i][1].posi = bottomright;
    obj->faces[i][1].texi = 2;
    obj->faces[i][2].posi = bottomleft2;
    obj->faces[i][2].texi = 1;
    i++;
    obj->faces[i][0].posi = bottomleft2;
    obj->faces[i][0].texi = 1;
    obj->faces[i][1].posi = bottomright;
    obj->faces[i][1].texi = 2;
    obj->faces[i][2].posi = bottomright2;
    obj->faces[i][2].texi = 3;
    i++;
  }
}

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
        if (pi >= obj->numvertices || ti >= obj->numvertices) {
          continue;
        }
        obj->vertices[pi].y =
            obj->vertices[ti].y +
            slopiness * ((mfloat_t)rand() / RAND_MAX - downbias);
      }
    }
  }
}

void haloo3d_gen_crossquad(haloo3d_obj *obj, haloo3d_fb *fb) {
  struct vec2 dims;
  uint16_t width = fb->width;
  uint16_t height = fb->height;
  if (width > height) {
    dims.x = 1.0;
    dims.y = (mfloat_t)height / width;
  } else {
    dims.x = (mfloat_t)width / height;
    dims.y = 1.0;
  }
  haloo3d_gen_obj_prealloc(obj, 8, 4, 4);
  // There's only 4 vtextures, as usual
  haloo3d_gen_boxvtexture(obj->vtexture);
  // We create two quads each, first on the x axis = 0 then on z = 0.
  // Order is topleft, topright, bottomleft, bottomright
  vec4(obj->vertices[0].v, -dims.x, dims.y, 0, 1);
  vec4(obj->vertices[1].v, dims.x, dims.y, 0, 1);
  vec4(obj->vertices[2].v, -dims.x, -dims.y, 0, 1);
  vec4(obj->vertices[3].v, dims.x, -dims.y, 0, 1);
  // then the x aligned one, same order
  vec4(obj->vertices[4].v, 0, dims.y, -dims.x, 1);
  vec4(obj->vertices[5].v, 0, dims.y, dims.x, 1);
  vec4(obj->vertices[6].v, 0, -dims.y, -dims.x, 1);
  vec4(obj->vertices[7].v, 0, -dims.y, dims.x, 1);
  // Only four faces. Do two per quad (iterate over quads)
  for (int i = 0; i < 2; i++) {
    obj->faces[i * 2][0] = (haloo3d_vertexi){.posi = 0 + i * 4, .texi = 1};
    obj->faces[i * 2][1] = (haloo3d_vertexi){.posi = 2 + i * 4, .texi = 0};
    obj->faces[i * 2][2] = (haloo3d_vertexi){.posi = 1 + i * 4, .texi = 3};
    obj->faces[i * 2 + 1][0] = (haloo3d_vertexi){.posi = 1 + i * 4, .texi = 3};
    obj->faces[i * 2 + 1][1] = (haloo3d_vertexi){.posi = 2 + i * 4, .texi = 0};
    obj->faces[i * 2 + 1][2] = (haloo3d_vertexi){.posi = 3 + i * 4, .texi = 2};
  }
}
