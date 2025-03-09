#include "haloo3d_unigi.h"
#include "haloo3d_ex.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

// ===========================================
// |                 IMAGE                   |
// ===========================================

void h3d_fb_writeppm(h3d_fb *fb, FILE *f) {
  fprintf(f, "P6 %d %d 15\n", fb->width, fb->height);
  uint8_t color[3];
  for (int i = 0; i < H3D_FB_SIZE(fb); i++) {
    uint16_t bc = fb->buffer[i];
    color[0] = (bc >> 8) & 0xF; // H3DC_R(bc);
    color[1] = (bc >> 4) & 0xf; // H3DC_G(bc);
    color[2] = bc & 0xf;        // H3DC_B(bc);
    fwrite(color, sizeof(uint8_t), 3, f);
  }
}

void h3d_fb_loadppm(FILE *f, h3d_fb *fb) {
  char tmp[4096];
  // Must ALWAYS start with "P6"
  int scanned = fscanf(f, "%4095s", tmp);
  if (scanned != 1 || strcmp(tmp, "P6") != 0) {
    dieerr("Image file not in P6 format (no P6 identifier)");
  }
  // Now just pull three digits
  int vals[3];
  int numvals = 0;
  while (numvals != 3) {
    scanned = fscanf(f, "%d", vals + numvals);
    if (scanned != 1) {
      // This might just be a comment. Consume the rest of the line if so
      scanned = fscanf(f, "%4095s", tmp);
      if (scanned != 1 || tmp[0] != '#' || !fgets(tmp, 4095, f)) {
        dieerr("Image file not in P6 format (unexpected header value: %s)",
               tmp);
      }
    } else {
      numvals++;
    }
  }
  // Consume one character, it's the whitespace after depth
  fgetc(f);
  fb->width = vals[0];
  fb->height = vals[1];
  int depth = vals[2];
  H3D_FB_TEXINIT(fb, fb->width, fb->height);
  // Must set everything to 0
  memset(fb->buffer, 0, H3D_FB_SIZE(fb));
  // Now let's just read until the end!
  int b = 0;
  int i = 0;
  int c = 0;
  while ((c = fgetc(f)) != EOF) {
    fb->buffer[i] |=
        0xF000 | ((uint16_t)((c / (float)depth) * 15 + 0.5) << ((2 - b) * 4));
    b++;
    if (b == 3) { // We've read the full rgb
      i++;
      b = 0;
    }
  }
}

void h3d_fb_writeppmfile(h3d_fb *fb, char *filename) {
  // And now we should be able to save the framebuffer
  FILE *f = fopen(filename, "w");
  if (f == NULL) {
    dieerr("Can't open %s for writing ppm image\n", filename);
  }
  h3d_fb_writeppm(fb, f);
  fclose(f);
  eprintf("Wrote ppm image to %s\n", filename);
}

void h3d_fb_loadppmfile(h3d_fb *tex, char *filename) {
  // Open a simple file and read the ppm from it
  FILE *f = fopen(filename, "r");
  if (f == NULL) {
    dieerr("Can't open %s for ppm image reading\n", filename);
  }
  h3d_fb_loadppm(f, tex); // This also calls init so you have to free
  fclose(f);
  eprintf("Read ppm image from %s\n", filename);
}

// ===========================================
// |              FRAMEBUFFER                |
// ===========================================

void h3d_fb_init(h3d_fb *fb, uint16_t width, uint16_t height) {
  H3D_FB_INIT(fb, width, height, 2);
}

void h3d_fb_free(h3d_fb *fb) { H3D_FB_FREE(fb); }

void h3d_sprite(h3d_fb *fb, h3d_fb *sprite, h3d_recti texrect,
                h3d_recti outrect) {
  // Precalc the step, as it's always the same even if we clip the rect
  const int FIXEDPOINTDEPTH = 16;
  int32_t stepx = (1 << FIXEDPOINTDEPTH) * (float)abs(texrect.x2 - texrect.x1) /
                  abs(outrect.x2 - outrect.x1);
  int32_t stepy = (1 << FIXEDPOINTDEPTH) * (float)abs(texrect.y2 - texrect.y1) /
                  abs(outrect.y2 - outrect.y1);
  int32_t texx = (1 << FIXEDPOINTDEPTH) * texrect.x1;
  int32_t texy = (1 << FIXEDPOINTDEPTH) * texrect.y1;
  // Clip the rect
  if (outrect.x1 < 0) {
    texx += stepx * -outrect.x1;
    outrect.x1 = 0;
  }
  if (outrect.y1 < 0) {
    texy += stepy * -outrect.y1;
    outrect.y1 = 0;
  }
  if (outrect.x2 >= fb->width) {
    outrect.x2 = fb->width - 1;
  }
  if (outrect.y2 >= fb->height) {
    outrect.y2 = fb->height - 1;
  }
  for (int y = outrect.y1; y < outrect.y2; y++) {
    texx = texrect.x1;
    for (int x = outrect.x1; x < outrect.x2; x++) {
      uint16_t pix =
          H3D_FB_GET(sprite, texx >> FIXEDPOINTDEPTH, texy >> FIXEDPOINTDEPTH);
      if (pix & 0xF000) {
        H3D_FB_SET(fb, x, y, pix);
      }
      texx += stepx;
    }
    texy += stepy;
  }
}

// ===========================================
// |              EASYSYS                    |
// ===========================================

void h3d_easystore_init(h3d_easystore *s) {
  for (int i = 0; i < H3D_EASYSTORE_MAX; i++) {
    s->objkeys[i][0] = 0;
    s->texkeys[i][0] = 0;
  }
}

#define _H3D_ES_CHECKKEY(key)                                                  \
  if (strlen(key) >= H3D_EASYSTORE_MAXKEY) {                                   \
    dieerr("Key too long! Max: %d\n", H3D_EASYSTORE_MAXKEY - 1);               \
  }
#define _H3D_ES_FOREACH(i) for (int i = 0; i < H3D_EASYSTORE_MAX; i++)
#define _H3D_ES_EMPTY(f) if (f[0] == 0)
#define _H3D_ES_NOTEMPTY(f) if (f[0] != 0)
#define _H3D_ES_FIND(f, key) if (strcmp(f, key) == 0)
#define _H3D_ES_NOEMPTY()                                                      \
  dieerr("No more room for objects! Max: %d\n", H3D_EASYSTORE_MAX);
#define _H3D_ES_NOFIND(key) dieerr("Object not found: %s\n", key);

h3d_obj *h3d_easystore_addobj(h3d_easystore *s, const char *key) {
  _H3D_ES_CHECKKEY(key);
  _H3D_ES_FOREACH(i) {
    _H3D_ES_EMPTY(s->objkeys[i]) {
      strcpy(s->objkeys[i], key);
      return s->_objects + i;
    }
  }
  _H3D_ES_NOEMPTY()
}

h3d_obj *h3d_easystore_getobj(h3d_easystore *s, const char *key) {
  _H3D_ES_CHECKKEY(key);
  _H3D_ES_FOREACH(i) {
    _H3D_ES_FIND(s->objkeys[i], key) { return s->_objects + i; }
  }
  _H3D_ES_NOFIND(key);
}

void h3d_easystore_deleteobj(h3d_easystore *s, const char *key,
                             void (*ondelete)(h3d_obj *)) {
  _H3D_ES_CHECKKEY(key);
  _H3D_ES_FOREACH(i) {
    _H3D_ES_FIND(s->objkeys[i], key) {
      s->objkeys[i][0] = 0;
      if (ondelete != NULL) {
        ondelete(s->_objects + i);
      }
      return;
    }
  }
  _H3D_ES_NOFIND(key);
}

void h3d_easystore_deleteallobj(h3d_easystore *s, void (*ondelete)(h3d_obj *)) {
  _H3D_ES_FOREACH(i) {
    _H3D_ES_NOTEMPTY(s->objkeys[i]) {
      s->objkeys[i][0] = 0;
      if (ondelete != NULL) {
        ondelete(s->_objects + i);
      }
    }
  }
}

h3d_fb *h3d_easystore_addtex(h3d_easystore *s, const char *key) {
  _H3D_ES_CHECKKEY(key);
  _H3D_ES_FOREACH(i) {
    _H3D_ES_EMPTY(s->texkeys[i]) {
      strcpy(s->texkeys[i], key);
      return s->_textures + i;
    }
  }
  _H3D_ES_NOEMPTY()
}

h3d_fb *h3d_easystore_gettex(h3d_easystore *s, const char *key) {
  _H3D_ES_CHECKKEY(key);
  _H3D_ES_FOREACH(i) {
    _H3D_ES_FIND(s->texkeys[i], key) { return s->_textures + i; }
  }
  _H3D_ES_NOFIND(key);
}

void h3d_easystore_deletetex(h3d_easystore *s, const char *key,
                             void (*ondelete)(h3d_fb *)) {
  _H3D_ES_CHECKKEY(key);
  _H3D_ES_FOREACH(i) {
    _H3D_ES_FIND(s->texkeys[i], key) {
      s->texkeys[i][0] = 0;
      if (ondelete != NULL) {
        ondelete(s->_textures + i);
      }
      return;
    }
  }
  _H3D_ES_NOFIND(key);
}

void h3d_easystore_deletealltex(h3d_easystore *s, void (*ondelete)(h3d_fb *)) {
  _H3D_ES_FOREACH(i) {
    _H3D_ES_NOTEMPTY(s->texkeys[i]) {
      s->texkeys[i][0] = 0;
      if (ondelete != NULL) {
        ondelete(s->_textures + i);
      }
    }
  }
}

void h3d_easytimer_init(h3d_easytimer *t, float avgweight) {
  t->sum = 0;
  t->last = 0;
  t->avgweight = avgweight;
  t->min = 99999999.0;
  t->max = 0.0;
}

void h3d_easytimer_start(h3d_easytimer *t) { t->start = clock(); }

void h3d_easytimer_end(h3d_easytimer *t) {
  clock_t end = clock();
  t->last = (float)(end - t->start) / CLOCKS_PER_SEC;
  if (t->sum == H3DVF(0))
    t->sum = t->last;
  t->sum = t->avgweight * t->sum + (H3DVF(1) - t->avgweight) * t->last;
  if (t->sum < t->min)
    t->min = t->sum;
  if (t->sum > t->max)
    t->max = t->sum;
}

// ===========================================
// |              GENERATION                 |
// ===========================================

// 4x4 dither patterns from 0 (no fill) to 16 (high fill). The pattern is
// actually 8x4 to make storage easier (each byte is 8 across)
// clang-format off
uint8_t _dither4x4[] = {
    0x00, 0x00, 0x00, 0x00,
    0x88, 0x00, 0x00, 0x00,
    0x88, 0x00, 0x22, 0x00,
    0xAA, 0x00, 0x22, 0x00,
    0xAA, 0x00, 0xAA, 0x00,
    0xAA, 0x44, 0xAA, 0x00,
    0xAA, 0x44, 0xAA, 0x11,
    0xAA, 0x55, 0xAA, 0x11,
    0xAA, 0x55, 0xAA, 0x55,
    0xEE, 0x55, 0xAA, 0x55,
    0xEE, 0x55, 0xBB, 0x55,
    0xFF, 0x55, 0xBB, 0x55,
    0xFF, 0x55, 0xFF, 0x55,
    0xFF, 0xDD, 0xFF, 0x55,
    0xFF, 0xDD, 0xFF, 0x77,
    0xFF, 0xFF, 0xFF, 0x77,
    0xFF, 0xFF, 0xFF, 0xFF,
};

inline int h3d_4x4dither_index(float dither) {
  if (dither < 0) {
    return 0;
  } else if (dither >= 1) {
    return 16 << 2;
  } else {
    return ((int)round(H3DVF(16) * dither) << 2);
  }
}

inline void h3d_getdither4x4(float dither, uint8_t *buf) {
  int index = h3d_4x4dither_index(dither);
  memcpy(buf, _dither4x4 + index, 4);
}

void h3d_apply_alternating(h3d_fb *fb, const uint16_t *cols, uint16_t numcols) {
  for (int y = 0; y < fb->height; y++) {
    for (int x = 0; x < fb->width; x++) {
      uint16_t basecol = H3D_FB_GET(fb, x, y);
      H3D_FB_SET(fb, x, y, h3d_col_blend(cols[(x + y) % numcols], basecol));
    }
  }
}

void h3d_apply_vgradient(h3d_fb *fb, uint16_t top, uint16_t bottom) {
  for (int y = 0; y < fb->height; y++) {
    uint16_t col = h3d_col_lerp(top, bottom, (hfloat_t)y / (fb->height - 1));
    for (int x = 0; x < fb->width; x++) {
      uint16_t basecol = H3D_FB_GET(fb, x, y);
      H3D_FB_SET(fb, x, y, h3d_col_blend(col, basecol));
    }
  }
}

// void h3d_apply_noise(h3d_fb *fb, float *noise, float scale) {
//   static int noisenext = 0;
//   int malloced = 0;
//   if (noise == NULL) {
//     mallocordie(noise, sizeof(float) * fb->width * fb->height);
//     fnl_state ns = fnlCreateState();
//     ns.noise_type = FNL_NOISE_OPENSIMPLEX2;
//     ns.seed = noisenext++;
//     ns.frequency = 1;
//     for (int y = 0; y < fb->height; y++) {
//       for (int x = 0; x < fb->width; x++) {
//         noise[x + y * fb->width] = fnlGetNoise2D(&ns, x, y);
//       }
//     }
//   }
//   uint16_t noisealpha = scale * 15;
//   for (int y = 0; y < fb->height; y++) {
//     for (int x = 0; x < fb->width; x++) {
//       uint16_t basecol = haloo3d_fb_get(fb, x, y);
//       uint16_t noisecolv = (noise[x + y * fb->width] + 1.0) / 2.0 * 15;
//       uint16_t noisecol =
//           H3DC_ARGB(noisealpha, noisecolv, noisecolv, noisecolv);
//       haloo3d_fb_set(fb, x, y, haloo3d_col_blend(noisecol, basecol));
//     }
//   }
//   if (malloced) {
//     free(noise);
//   }
// }

void h3d_apply_brick(h3d_fb *fb, uint16_t width, uint16_t height,
                     uint16_t color) {
  int i = 0;
  for (int y = height - 1; y < fb->height + height; y += height) {
    int yofs = (width * (2 + 5 * (i & 1))) / 10;
    for (int x = 0; x < fb->width; x++) {
      if (y < fb->height) {
        uint16_t basecol = H3D_FB_GET(fb, x, y);
        H3D_FB_SET(fb, x, y, h3d_col_blend(color, basecol));
      }
      if (x == yofs) {
        for (int v = y - (height - 1); v < y; v++) {
          if (v < fb->height) {
            uint16_t basecol = H3D_FB_GET(fb, x, v);
            H3D_FB_SET(fb, x, v, h3d_col_blend(color, basecol));
          }
        }
        yofs += width;
      }
    }
    i++;
  }
}

void h3d_apply_rect(h3d_fb *fb, h3d_recti rect, uint16_t color, int width) {
  uint16_t basecol;
  for (int i = 0; i < width; i++) {
    for (int x = rect.x1; x < rect.x2; x++) {
      // Top and bottom
      basecol = H3D_FB_GET(fb, x, rect.y1 + i);
      H3D_FB_SET(fb, x, rect.y1 + i, h3d_col_blend(color, basecol));
      basecol = H3D_FB_GET(fb, x, rect.y2 - i - 1);
      H3D_FB_SET(fb, x, rect.y2 - i - 1, h3d_col_blend(color, basecol));
    }
    for (int y = rect.y1; y < rect.y2; y++) {
      // left and right
      basecol = H3D_FB_GET(fb, rect.x1 + i, y);
      H3D_FB_SET(fb, rect.x1 + i, y, h3d_col_blend(color, basecol));
      basecol = H3D_FB_GET(fb, rect.x2 - i - 1, y);
      H3D_FB_SET(fb, rect.x2 - i - 1, y, h3d_col_blend(color, basecol));
    }
  }
}

// Fill rectangle, EXCLUSIVE
void h3d_apply_fillrect(h3d_fb *fb, h3d_recti rect, uint16_t color,
                        const uint8_t dithering[4]) {
  for (int y = rect.y1; y < rect.y2; y++) {
    uint8_t dither = dithering[y & 3];
    for (int x = rect.x1; x < rect.x2; x++) {
      if (dither & (1 << (x & 7))) {
        uint16_t basecol = H3D_FB_GET(fb, x, y);
        H3D_FB_SET(fb, x, y, h3d_col_blend(color, basecol));
      }
    }
  }
}

// Create a NEW texture that is entirely a solid color. Underneath, this will
// use a tiny 1x1 texture, that you'll still unfortunately need to free.
void h3d_gen_solidtex(h3d_fb *fb, uint16_t color) {
  H3D_FB_TEXINIT(fb, 1, 1);
  H3D_FB_SET(fb, 0, 0, color);
}

void h3d_gen_palettetex(h3d_fb *fb) {
  H3D_FB_TEXINIT(fb, 64, 64);
  for (int i = 0; i < 64 * 64; i++) {
    fb->buffer[i] = 0xF000 | i;
  }
}

void h3d_gen_paletteuv(uint16_t col, vec3 result) {
  VEC3(result, (0.5 + (col & 63)) / 64, (0.5 + ((col & 0xFFF) >> 6)) / 64, 1.0);
}

// Only for these functions
void haloo3d_gen_obj_prealloc(h3d_obj *obj, uint16_t numverts, uint16_t numvtex,
                              uint16_t numfaces) {
  obj->numvertices = numverts;
  obj->numvtextures = numvtex;
  obj->numfaces = numfaces;
  mallocordie(obj->vertices, obj->numvertices * sizeof(vec4));
  mallocordie(obj->vtexture, obj->numvtextures * sizeof(vec3));
  mallocordie(obj->faces, obj->numfaces * sizeof(h3d_objface));
}

void h3d_gen_boxvtexture(vec3 *textures) {
  VEC3(textures[0], 0.001, 0.001, 0); // Oops, it sometimes overflows
  VEC3(textures[1], 0.001, 0.999, 0);
  VEC3(textures[2], 0.999, 0.001, 0); // Oops, it sometimes overflows
  VEC3(textures[3], 0.999, 0.999, 0);
}

// This technically inits the obj, so you will need to free it
void h3d_gen_skybox(h3d_obj *obj) {
  haloo3d_gen_obj_prealloc(obj, 8, 4, 12);
  // There are only four corners of a texture and we're making a box... yeah
  h3d_gen_boxvtexture(obj->vtexture);
  // Cube faces are weird, I guess just manually do them? ugh
  // First 4 are the bottom vertices. We can make two faces out of these
  VEC4(obj->vertices[0], -1, -1, -1, 1);
  VEC4(obj->vertices[1], 1, -1, -1, 1);
  VEC4(obj->vertices[2], 1, -1, 1, 1);
  VEC4(obj->vertices[3], -1, -1, 1, 1);
  // Now the top 4 vertices, same order as bottom
  VEC4(obj->vertices[4], -1, 1, -1, 1);
  VEC4(obj->vertices[5], 1, 1, -1, 1);
  VEC4(obj->vertices[6], 1, 1, 1, 1);
  VEC4(obj->vertices[7], -1, 1, 1, 1);
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
      obj->faces[fi][v].verti = fv[fi][v];
      obj->faces[fi][v].texi = vt[fi][v];
    }
  }
}

void h3d_gen_plane(h3d_obj *obj, uint16_t size) {
  haloo3d_gen_obj_prealloc(obj, (size + 1) * (size + 1), 4, 2 * size * size);
  // Vtexture is just the four corners again
  h3d_gen_boxvtexture(obj->vtexture);
  // Generate all the simple vertices along the plane at y=0
  int i = 0;
  for (hfloat_t z = -size / 2.0; z <= size / 2.0; z += 1) {
    for (hfloat_t x = -size / 2.0; x <= size / 2.0; x += 1) {
      VEC4(obj->vertices[i], x, 0, z, 1);
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
      obj->faces[i][0].verti = topleft;
      obj->faces[i][0].texi = 1;
      obj->faces[i][1].verti = bottomleft;
      obj->faces[i][1].texi = 0;
      obj->faces[i][2].verti = topright;
      obj->faces[i][2].texi = 3;
      i++;
      obj->faces[i][0].verti = topright;
      obj->faces[i][0].texi = 3;
      obj->faces[i][1].verti = bottomleft;
      obj->faces[i][1].texi = 0;
      obj->faces[i][2].verti = bottomright;
      obj->faces[i][2].texi = 2;
      i++;
    }
  }
  eprintf("Generated plane with %d vertices, %d faces\n", obj->numvertices,
          obj->numfaces);
}

// Generate a face at the given cell in the given direction.
void h3d_gen_grid_quad(h3d_obj *obj, int x, int y, const vec2i dir) {
  if (dir[H3DX] == 1) {
    vec2i dir2;
    dir2[H3DY] = dir[H3DY];
    dir2[H3DX] = -1;
    h3d_gen_grid_quad(obj, x + 1, y, dir2);
    return;
  }
  if (dir[H3DY] == 1) {
    vec2i dir2;
    dir2[H3DX] = dir[H3DX];
    dir2[H3DY] = -1;
    h3d_gen_grid_quad(obj, x, y + 1, dir2);
    return;
  }
  const int vertplanesize = obj->numvertices / 2;
  const int vertplanewidth = sqrt(vertplanesize);

  int topleft = x + y * vertplanewidth;
  int topright = x + 1 + y * vertplanewidth;
  int bottomleft = x + (y + 1) * vertplanewidth;
  int topleft2 = topleft + vertplanesize;
  int topright2 = topright + vertplanesize;
  int bottomleft2 = bottomleft + vertplanesize;

  int i = obj->numfaces;

  // We only do south (negative?) and west (negative) walls?
  if (dir[H3DY] == -1) {
    // remember to wind counter-clockwise
    obj->faces[i][0].verti = topleft;
    obj->faces[i][0].texi = 0;
    obj->faces[i][1].verti = topright;
    obj->faces[i][1].texi = 2;
    obj->faces[i][2].verti = topleft2;
    obj->faces[i][2].texi = 1;
    i++;
    obj->faces[i][0].verti = topleft2;
    obj->faces[i][0].texi = 1;
    obj->faces[i][1].verti = topright;
    obj->faces[i][1].texi = 2;
    obj->faces[i][2].verti = topright2;
    obj->faces[i][2].texi = 3;
    obj->numfaces += 2;
  } else if (dir[H3DX] == -1) {
    obj->faces[i][0].verti = bottomleft;
    obj->faces[i][0].texi = 0;
    obj->faces[i][1].verti = topleft;
    obj->faces[i][1].texi = 2;
    obj->faces[i][2].verti = bottomleft2;
    obj->faces[i][2].texi = 1;
    i++;
    obj->faces[i][0].verti = bottomleft2;
    obj->faces[i][0].texi = 1;
    obj->faces[i][1].verti = topleft;
    obj->faces[i][1].texi = 2;
    obj->faces[i][2].verti = topleft2;
    obj->faces[i][2].texi = 3;
    obj->numfaces += 2;
  }
}

// Generate a basic grid of given size. Expect walls to be viewed from both
// sides. Put enough room for faces, even if they're not used (yes it's
// wasteful)
void h3d_gen_grid(h3d_obj *obj, uint16_t size, uint8_t faces) {
  haloo3d_gen_obj_prealloc(obj, 2 * (size + 1) * (size + 1), 4,
                           4 * size * (size + 1));
  // Vtexture is just the four corners again
  h3d_gen_boxvtexture(obj->vtexture);
  // Generate the bottom vertices. It's the same as the plane
  int i = 0;
  for (hfloat_t z = -size / 2.0; z <= size / 2.0; z += 1) {
    for (hfloat_t x = -size / 2.0; x <= size / 2.0; x += 1) {
      VEC4(obj->vertices[i], x, 0, z, 1);
      i++;
    }
  }
  // Generate the top vertices. It's the same as the plane
  for (hfloat_t z = -size / 2.0; z <= size / 2.0; z += 1) {
    for (hfloat_t x = -size / 2.0; x <= size / 2.0; x += 1) {
      VEC4(obj->vertices[i], x, 1, z, 1);
      i++;
    }
  }
  obj->numfaces = 0;
  if (!faces) {
    return;
  }
  // Now go crazy and generate the monstrous amount of faces.
  // First is the main grid of left and top for each cell
  int32_t dir[2];
  // struct vec2i dir;
  for (int z = 0; z < size; z++) {
    for (int x = 0; x < size; x++) {
      dir[H3DX] = -1;
      dir[H3DY] = 0;
      h3d_gen_grid_quad(obj, x, z, dir);
      dir[H3DX] = 0;
      dir[H3DY] = -1;
      h3d_gen_grid_quad(obj, x, z, dir);
    }
  }
  // And then we generate some walls along the edges we didn't finish
  for (int i = 0; i < size; i++) {
    dir[H3DX] = -1;
    dir[H3DY] = 0;
    h3d_gen_grid_quad(obj, size, i, dir);
    dir[H3DX] = 0;
    dir[H3DY] = -1;
    h3d_gen_grid_quad(obj, i, size, dir);
  }
}

void h3d_gen_sloped(h3d_obj *obj, uint16_t size, hfloat_t slopiness,
                    hfloat_t downbias) {
  h3d_gen_plane(obj, size);
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
        obj->vertices[pi][H3DY] =
            obj->vertices[ti][H3DY] +
            slopiness * ((hfloat_t)rand() / (hfloat_t)RAND_MAX - downbias);
      }
    }
  }
}

void h3d_gen_crossquad_generic(h3d_obj *obj, const h3d_fb *fb,
                               const vec3 center, int count) {
  hfloat_t dims[2];
  uint16_t width = fb->width;
  uint16_t height = fb->height;
  if (width > height) {
    dims[H3DX] = 1.0;
    dims[H3DY] = (hfloat_t)height / width;
  } else {
    dims[H3DX] = (hfloat_t)width / height;
    dims[H3DY] = 1.0;
  }
  haloo3d_gen_obj_prealloc(obj, 4 * count, 4, 2 * count);
  // There's only 4 vtextures, as usual
  h3d_gen_boxvtexture(obj->vtexture);
  // We create two quads each, first on the x axis = 0 then on z = 0.
  // Order is topleft, topright, bottomleft, bottomright
  VEC4(obj->vertices[0], center[H3DX] - dims[H3DX], center[H3DY] + dims[H3DY],
       center[H3DZ], 1);
  VEC4(obj->vertices[1], center[H3DX] + dims[H3DX], center[H3DY] + dims[H3DY],
       center[H3DZ], 1);
  VEC4(obj->vertices[2], center[H3DX] - dims[H3DX], center[H3DY] - dims[H3DY],
       center[H3DZ], 1);
  VEC4(obj->vertices[3], center[H3DX] + dims[H3DX], center[H3DY] - dims[H3DY],
       center[H3DZ], 1);
  if (count == 2) {
    // then the x aligned one, same order
    VEC4(obj->vertices[4], center[H3DX], center[H3DY] + dims[H3DY],
         center[H3DZ] - dims[H3DX], 1);
    VEC4(obj->vertices[5], center[H3DX], center[H3DY] + dims[H3DY],
         center[H3DZ] + dims[H3DX], 1);
    VEC4(obj->vertices[6], center[H3DX], center[H3DY] - dims[H3DY],
         center[H3DZ] - dims[H3DX], 1);
    VEC4(obj->vertices[7], center[H3DX], center[H3DY] - dims[H3DY],
         center[H3DZ] + dims[H3DX], 1);
  }
  // Only four faces. Do two per quad (iterate over quads)
  for (int i = 0; i < count; i++) {
    // topleft
    obj->faces[i * 2][0].verti = 0 + i * 4;
    obj->faces[i * 2][0].texi = 1;
    // bottomleft
    obj->faces[i * 2][1].verti = 2 + i * 4;
    obj->faces[i * 2][1].texi = 0;
    // topright
    obj->faces[i * 2][2].verti = 1 + i * 4;
    obj->faces[i * 2][2].texi = 3;
    obj->faces[i * 2 + 1][0].verti = 1 + i * 4;
    obj->faces[i * 2 + 1][0].texi = 3;
    obj->faces[i * 2 + 1][1].verti = 2 + i * 4;
    obj->faces[i * 2 + 1][1].texi = 0;
    obj->faces[i * 2 + 1][2].verti = 3 + i * 4;
    obj->faces[i * 2 + 1][2].texi = 2;
  }
}

void h3d_gen_crossquad(h3d_obj *obj, const h3d_fb *fb, const vec3 center) {
  h3d_gen_crossquad_generic(obj, fb, center, 2);
}

void h3d_gen_quad(h3d_obj *obj, const h3d_fb *fb, const vec3 center) {
  h3d_gen_crossquad_generic(obj, fb, center, 1);
}

void h3d_gen_gradient(h3d_fb *buf, uint16_t topcol, uint16_t botcol,
                      int height) {
  // Precalc an array indicating the bands of color.
  int row[64]; // not sure how many bands there can be but...
  uint16_t col[64];
  int bandscount = 1;
  row[0] = 0;
  col[0] = topcol;
  uint8_t dither[4];
  for (int y = 1; y < height; y++) {
    uint16_t thiscol = h3d_col_lerp(topcol, botcol, (float)y / (height - 1));
    if (thiscol != col[bandscount - 1]) {
      row[bandscount] = y;
      col[bandscount] = thiscol;
      bandscount++;
    }
  }
  for (int band = 0; band < bandscount - 1; band++) {
    for (int r = row[band]; r < row[band + 1]; r++) {
      h3d_getdither4x4((float)(r - row[band]) / (row[band + 1] - row[band]),
                       dither);
      uint8_t df = dither[r & 3];
      for (int b = 0; b < 8; b++) {
        H3D_FB_SET(buf, b, r, (df & 1) ? col[band + 1] : col[band]);
        df >>= 1;
      }
      uint16_t *bufstart = buf->buffer + r * buf->width;
      // Then, a repeated growing copy to minimize copies? IDK
      for (int size = 8; size < buf->width; size <<= 1) {
        memcpy(bufstart + size, bufstart,
               sizeof(uint16_t) * H3D_MIN(size, buf->width - size));
      }
    }
  }
}

// ===========================================
// |          PRINTING (legacy)              |
// ===========================================

// These are 8x8 glyphs for characters in the ascii range. If you try to
// print utf8, oh well
// clang-format off
const uint64_t haloo3d_print_basic_glyphs[256] = {
	9179274238964760320, 9179274238964760320, 9179274238964760320, 9179274238964760320,
	9179274238964760320, 9179274238964760320, 9179274238964760320, 9179274238964760320,
	9179274238964760320, 9179274238964760320, 0, 9179274238964760320,
	9179274238964760320, 9179274238964760320, 9179274238964760320, 9179274238964760320,
	9179274238964760320, 9179274238964760320, 9179274238964760320, 9179274238964760320,
	9179274238964760320, 9179274238964760320, 9179274238964760320, 9179274238964760320,
	9179274238964760320, 9179274238964760320, 9179274238964760320, 9179274238964760320,
	9179274238964760320, 9179274238964760320, 9179274238964760320, 9179274238964760320,
	0, 864704451948973056, 77916943872, 3926917006560802304,
	594308291526264832, 2546271121760928256, 4495563687961759232, 17314876416,
	4038609588785526784, 1015614699495493120, 907818496, 578780756331986944,
	290495421644537856, 68169720922112, 868068828175663104, 72908667983241216,
	4496672031474138624, 9086038739982491136, 9152165585887641088, 4495542665477307904,
	6944690690302179328, 4495542819536207616, 4495545975336812032, 217874930004623104,
	4495545972652457472, 4495542948995481088, 868068879916597248, 579838503764885504,
	4043122079833260032, 17451714844033024, 1016749309024010240, 864704581909822976,
	4468480587830869504, 7161707897292200960, 4567603570985352960, 4495440164522507776,
	2248249917505216256, 9152162299476410112, 217020638773346048, 9107232251455962624,
	7161677231228674816, 9086038739982515712, 4495546118630832128, 7148086785261921024,
	9152162179217294080, 7161677145800401664, 7161694806436438784, 4495546131566247424,
	217020777829187328, 6787904730176896512, 7148086992631447296, 4495542672506961408,
	1736164148113866240, 4495546131566256896, 584401852148441856, 7161699221595448064,
	7167265623011451648, 1736164304046417408, 9153300282022592256, 4469266303152438784,
	4638760496030679552, 4481134612758674944, 426109569024, 9151314442816847872,
	525312, 9107261823295422464, 4567603724993889024, 4495440577822720000,
	9107232150446432256, 4468555319500341248, 868082075986312192, 4494731393896546304,
	7161677110359294720, 9158096334119507968, 4495542820089651200, 7148091339132044032,
	8650855245695615744, 7740398492928966656, 7161677110359097344, 4495546130938986496,
	217086902535192576, 6944689591186096128, 217020948950286336, 4566789675129241600,
	8938532608234228736, 4495546131559743488, 584401852141928448, 3926975509056978944,
	8592336193679523840, 1088805010407424000, 9155286121465249792, 4038616220315695104,
	868082074056920064, 1015588345173511680, 16725143816503296, 9179274238964760320,
	9179274238964760320, 9179274238964760320, 9179274238964760320, 9179274238964760320,
	9179274238964760320, 9179274238964760320, 9179274238964760320, 9179274238964760320,
	9179274238964760320, 9179274238964760320, 9179274238964760320, 9179274238964760320,
	9179274238964760320, 9179274238964760320, 9179274238964760320, 9179274238964760320,
	9179274238964760320, 9179274238964760320, 9179274238964760320, 9179274238964760320,
	9179274238964760320, 9179274238964760320, 9179274238964760320, 9179274238964760320,
	9179274238964760320, 9179274238964760320, 9179274238964760320, 9179274238964760320,
	9179274238964760320, 9179274238964760320, 9179274238964760320, 9179274238964760320,
	0, 873168492155636736, 594029897647523840, 9153010024071969792,
	9601021905076224, 593921263713805056, 868082022517312512, 4350616204763930112,
	13824, 4485969256336408064, 138965787098624, 7797449725392715776,
	13563841728217088, 0, 4485942902282862080, 15872,
	68613771050496, 4467579661845006336, 138590950735360, 68595868712448,
	528384, 217086903145685760, 2893611337457236992, 34359738368,
	290482175965396992, 138641948548608, 68596590132736, 1960873667403448320,
	4629831612383822336, 8074997297135682048, 4629827222927311616, 4495440280832448512,
	7169558215846790144, 7169558215846793216, 7169558215847577600, 7169558215847585792,
	7169558215846278656, 7169558215847582720, 8870718504632744960, 290550770944916992,
	9152192967623574528, 9152192967623577600, 9152192967624361984, 9152192967623063040,
	9159226650880640000, 9159226650880643072, 9159226650881427456, 9159226650880128512,
	2177040098522439168, 7166189523864659968, 4495546130939511808, 4495546130939514880,
	4495546130940299264, 4495546130940307456, 4495546130939000320, 2455596579386556416,
	4495550564240932352, 4495546131560268800, 4495546131560271872, 4495546131561056256,
	4495546131559757312, 2025562528994037760, 234014984164541184, 4281624976256343552,
	9107261823295947776, 9107261823295950848, 9107261823296735232, 9107261823296743424,
	9107261823295436288, 9107261823296740352, 4543427701976858624, 290620727373987840,
	4468555319500866560, 4468555319500869632, 4468555319501654016, 4468555319500355072,
	9158096334120027136, 9158096334120030208, 9158096334120814592, 9158096334119515648,
	4495546132015880192, 7161677110360418304, 4495545970985534464, 4495545970985537536,
	4495545970986321920, 4495545970986330112, 4495545970988548096, 576528922158563328,
	4496680895658328064, 4495546129899324416, 4495546129899327488, 4495546129900111872,
	4495546129902338048, 1088805010407952384, 217086902535389952, 1088805010407437824,
};
// clang-format on

void h3d_print_init(h3d_print_tracker *t, char *buf, int buflen, h3d_fb *fb) {
  t->scale = 1;
  t->bcolor = 0xF000;
  t->fcolor = 0xFFFF;
  t->glyphs = haloo3d_print_basic_glyphs;
  t->buffer = buf;
  t->buflen = buflen;
  t->logprints = 0;
  t->bounds.x1 = 0;
  t->bounds.y1 = 0;
  t->bounds.x2 = -1;
  t->bounds.y2 = -1;
  t->fb = fb;
  t->fast = 1;
  h3d_print_refresh(t);
}

void h3d_print_refresh(h3d_print_tracker *t) {
  t->x = H3D_MAX(0, t->bounds.x1);
  t->y = H3D_MAX(0, t->bounds.y1);
}

void h3d_print(h3d_print_tracker *t, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  vsnprintf(t->buffer, t->buflen, fmt, args);
  va_end(args);
  if (t->logprints) {
    eprintf("%s", t->buffer);
  }
  h3d_recti srect = {
      .x1 = 0,
      .y1 = 0,
      .x2 = t->scale * H3D_PRINT_CHW,
      .y2 = t->scale * H3D_PRINT_CHH,
  };
  h3d_recti trect = {
      .x1 = 0,
      .y1 = 0,
      .x2 = H3D_PRINT_CHW,
      .y2 = H3D_PRINT_CHH,
  };
  uint16_t sx = H3D_MAX(0, t->bounds.x1);
  uint16_t sy = H3D_MAX(0, t->bounds.y1);
  uint16_t ex = t->bounds.x2 < 0 ? t->fb->width : t->bounds.x2;
  uint16_t ey = t->bounds.y2 < 0 ? t->fb->height : t->bounds.y2;
  if (t->y < sy)
    t->y = sy;
  if (t->x < sx)
    t->x = sx;
  h3d_fb tex;
  uint8_t prerendered[256] = {0};
  //   32k on the stack! is this ok??
  uint16_t buffer[H3D_PRINT_CHW * H3D_PRINT_CHH * 256];
  tex.buffer = buffer;
  tex.width = H3D_PRINT_CHW;
  tex.height = H3D_PRINT_CHH;
  const int len = strlen(t->buffer);
  for (int i = 0; i < len; i++) {
    // eprintf("P[%d](%d,%d) : %c\n", i, t->x, t->y, t->buffer[i]);
    if (t->buffer[i] == '\n') {
      t->y += t->scale * H3D_PRINT_CHH;
      t->x = sx;
      continue;
    }
    // We don't support word wrap btw, just char wrap based on rect bounds. Our
    // int scaler is fast and thus has some problems, so we don't want to print
    // off screen
    if (t->x + t->scale * H3D_PRINT_CHW > ex) {
      t->y += t->scale * H3D_PRINT_CHH;
      t->x = sx;
    }
    if (t->y + t->scale * H3D_PRINT_CHH > ey) {
      break;
    }
    int glyph = t->buffer[i];
    tex.buffer = buffer + glyph * H3D_PRINT_CHW * H3D_PRINT_CHH;
    if (!prerendered[glyph]) {
      h3d_print_convertglyph(t->glyphs[glyph], t->bcolor, t->fcolor, &tex);
      prerendered[glyph] = 1;
    }
    if (t->fast) {
      h3d_fb_intscale(&tex, t->fb, t->x, t->y, t->scale);
    } else {
      srect.x1 = t->x;
      srect.y1 = t->y;
      srect.x2 = srect.x1 + t->scale * H3D_PRINT_CHW;
      srect.y2 = srect.y1 + t->scale * H3D_PRINT_CHH;
      h3d_sprite(t->fb, &tex, trect, srect);
    }
    t->x += t->scale * H3D_PRINT_CHW;
  }
}

void h3d_print_convertglyph(uint64_t glyph, uint16_t bcolor, uint16_t fcolor,
                            h3d_fb *out) {
  for (int i = 0; i < H3D_PRINT_CHW * H3D_PRINT_CHH; i++) {
    out->buffer[i] = (glyph & 1) ? fcolor : bcolor;
    glyph >>= 1;
  }
}
