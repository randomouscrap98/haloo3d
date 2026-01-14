#include "haloo3d_ex.h"

// ========================================
// |            IMAGE                     |
// ========================================

void h3d_fb_writeppm(h3d_fb *fb, FILE *f, h3d_fb_imgout cc) {
  uint8_t color[4];
  // Figure out max color with a bit of a hack
  cc(0xFFFF, color);
  uint8_t cwidth = color[1];
  fprintf(f, "P6 %d %d %d\n", fb->width, fb->height, cwidth);
  for (int i = 0; i < H3D_FB_SIZE(fb); i++) {
    cc(fb->buffer[i], color);
    if (!color[0])
      memset(color, 0, 4); // set to black if no alpha
    fwrite(color + 1, sizeof(uint8_t), 3, f);
  }
}

void h3d_fb_writeppmfile(h3d_fb *fb, char *filename, h3d_fb_imgout cc) {
  // And now we should be able to save the framebuffer
  FILE *f = fopen(filename, "w");
  if (f == NULL) {
    dieerr("Can't open %s for writing ppm image\n", filename);
  }
  h3d_fb_writeppm(fb, f, cc);
  fclose(f);
  eprintf("Wrote ppm image to %s\n", filename);
}

void h3d_fb_loadppm(FILE *f, h3d_fb *fb, h3d_fb_imgin cc) {
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
  float buf[4];
  buf[0] = 1; // Alpha always full in ppm
  while ((c = fgetc(f)) != EOF) {
    b++;
    buf[b] = c / (float)depth;
    if (b == 3) { // We've read the full rgb
      fb->buffer[i] = cc(buf);
      i++;
      b = 0;
    }
  }
}

void h3d_fb_loadppmfile(h3d_fb *tex, char *filename, h3d_fb_imgin cc) {
  // Open a simple file and read the ppm from it
  FILE *f = fopen(filename, "r");
  if (f == NULL) {
    dieerr("Can't open %s for ppm image reading\n", filename);
  }
  h3d_fb_loadppm(f, tex, cc); // This also calls init so you have to free
  fclose(f);
  eprintf("Read ppm image from %s\n", filename);
}

// ========================================
// |            FRAMEBUFFER               |
// ========================================

#define _H3D_FBF_BL(dbuf, sbuf)                                                \
  *dbuf = *sbuf;                                                               \
  dbuf++;

// Explicit loop unrolling for various amounts of scale factors
#define _H3D_FBF_ROW2(dbuf, sbuf, sbufe)                                       \
  while (sbuf < sbufe) {                                                       \
    _H3D_FBF_BL(dbuf, sbuf);                                                   \
    _H3D_FBF_BL(dbuf, sbuf);                                                   \
    sbuf++;                                                                    \
  }

#define _H3D_FBF_ROW3(dbuf, sbuf, sbufe)                                       \
  while (sbuf < sbufe) {                                                       \
    _H3D_FBF_BL(dbuf, sbuf);                                                   \
    _H3D_FBF_BL(dbuf, sbuf);                                                   \
    _H3D_FBF_BL(dbuf, sbuf);                                                   \
    sbuf++;                                                                    \
  }

#define _H3D_FBF_ROW4(dbuf, sbuf, sbufe)                                       \
  while (sbuf < sbufe) {                                                       \
    _H3D_FBF_BL(dbuf, sbuf);                                                   \
    _H3D_FBF_BL(dbuf, sbuf);                                                   \
    _H3D_FBF_BL(dbuf, sbuf);                                                   \
    _H3D_FBF_BL(dbuf, sbuf);                                                   \
    sbuf++;                                                                    \
  }

#define _H3D_FBF_ROWN(dbuf, sbuf, sbufe)                                       \
  while (sbuf < sbufe) {                                                       \
    for (int sx = 0; sx < scale; sx++) {                                       \
      _H3D_FBF_BL(dbuf, sbuf);                                                 \
    }                                                                          \
    sbuf++;                                                                    \
  }

// Fast draw the ENTIRE src into the dst at x, y at the given integer scale. If
// drawing outside the bounds or too big, behavior is undefined (for now). Does
// NOT check alpha, simply a full overwrite
void h3d_fb_intscale(h3d_fb *src, h3d_fb *dst, int dstofsx, int dstofsy,
                     uint8_t scale) {
  // calculate total usable width and height given offset
  const int dstwidth = dstofsx < 0 ? dst->width : dst->width - dstofsx;
  const int dstheight = dstofsy < 0 ? dst->height : dst->height - dstofsy;
  if (dstwidth < 0 || dstheight < 0) { // degenerate draw
    return;
  }
  // calculate actual width and height
  const int width = sizeof(uint16_t) * H3D_MIN(src->width * scale, dstwidth);
  const int height = H3D_MIN(src->height * scale, dstheight);
  // Special very fast case of scale 1
  if (scale == 1) {
    uint16_t *dbuf =
        dst->buffer + H3D_MAX(0, dstofsx) + dst->width * H3D_MAX(0, dstofsy);
    uint16_t *sbuf =
        src->buffer + H3D_MAX(0, -dstofsx) + src->width * H3D_MAX(0, -dstofsy);
    uint16_t *dbuf_e = dbuf + dst->width * height;
    while (dbuf < dbuf_e) {
      memcpy(dbuf, sbuf, width);
      sbuf += src->width;
      dbuf += dst->width;
    }
    return;
  }
  // Need a step per y of src and a step per y of dst
  uint16_t *dbuf_y = &dst->buffer[dstofsx + dstofsy * dst->width];
  uint16_t *sbuf_y = src->buffer;
  uint16_t *sbuf_ye = src->buffer + src->width * src->height;
  // Iterate over original image
  while (sbuf_y < sbuf_ye) {
    for (int sy = 0; sy < scale; sy++) {
      uint16_t *sbuf = sbuf_y;
      uint16_t *sbufe = sbuf_y + src->width;
      uint16_t *dbuf = dbuf_y;
      switch (scale) {
      case 2:
        _H3D_FBF_ROW2(dbuf, sbuf, sbufe);
        break;
      case 3:
        _H3D_FBF_ROW3(dbuf, sbuf, sbufe);
        break;
      case 4:
        _H3D_FBF_ROW4(dbuf, sbuf, sbufe);
        break;
      default:
        _H3D_FBF_ROWN(dbuf, sbuf, sbufe);
      }
      dbuf_y += dst->width;
    }
    sbuf_y += src->width;
  }
}

void h3d_fb_fill(h3d_fb *src, h3d_fb *dst, uint8_t centered) {
  uint16_t scalex = dst->width / src->width;
  uint16_t scaley = dst->height / src->height;
  uint16_t scale = scalex < scaley ? scalex : scaley;
  if (scale == 0) {
    scale = 1;
  }
  int newwidth = scale * src->width;
  int newheight = scale * src->height;
  int dstofsx = centered ? (dst->width - newwidth) >> 1 : 0;
  int dstofsy = centered ? (dst->height - newheight) >> 1 : 0;
  h3d_fb_intscale(src, dst, dstofsx, dstofsy, scale);
}

// ========================================
// |            OBJECT (MODEL)            |
// ========================================

void h3d_obj_init(h3d_obj *obj, uint16_t numf, uint16_t numv) {
  (obj)->numfaces = 0;
  (obj)->numvertices = 0;
  (obj)->numvtextures = 0;
  (obj)->numvnormals = 0;
  mallocordie((obj)->faces, sizeof(h3d_objface) * numf);
  mallocordie((obj)->vertices, sizeof(vec4) * numv);
  mallocordie((obj)->vtexture, sizeof(vec3) * numv);
  mallocordie((obj)->vnormals, sizeof(vec3) * numv);
}

void h3d_obj_initmax(h3d_obj *obj) {
  h3d_obj_init(obj, H3D_OBJ_MAXFACES, H3D_OBJ_MAXVERTICES);
}

void h3d_obj_free(h3d_obj *obj) {
  free((obj)->faces);
  free((obj)->vertices);
  free((obj)->vtexture);
  free((obj)->vnormals);
}

void h3d_obj_shrink(h3d_obj *obj) {
  reallocordie((obj)->faces, sizeof(h3d_objface) * H3D_MAX(1, (obj)->numfaces));
  reallocordie((obj)->vertices, sizeof(vec4) * H3D_MAX(1, (obj)->numvertices));
  reallocordie((obj)->vtexture, sizeof(vec3) * H3D_MAX(1, (obj)->numvtextures));
  reallocordie((obj)->vnormals, sizeof(vec3) * H3D_MAX(1, (obj)->numvnormals));
}

void h3d_obj_load(h3d_obj *obj, FILE *f) {
  h3d_obj_initmax(obj);
  char line[H3D_OBJ_MAXLINESIZE];
  char err[1024];
  while (fgets(line, H3D_OBJ_MAXLINESIZE, f)) {
    h3d_obj_parseline(obj, line, err, 1024);
  }
  h3d_obj_shrink(obj);
}

void h3d_obj_loadstring(h3d_obj *obj, const char *str) {
  h3d_obj_initmax(obj);
  char line[H3D_OBJ_MAXLINESIZE];
  char err[1024];
  char *endline;
  int running = 1;
  while (running) {
    endline = strchr(str, '\n');
    if (endline) {
      strncpy(line, str, H3D_MIN(H3D_OBJ_MAXLINESIZE - 1, endline - str));
      str = endline + 1;
    } else {
      strncpy(line, str, H3D_OBJ_MAXLINESIZE - 1);
      running = 0;
    }
    h3d_obj_parseline(obj, line, err, 1024);
  }
  h3d_obj_shrink(obj);
  eprintf("Read from object string: v=%d, f=%d, t=%d\n", (obj)->numvertices,
          (obj)->numfaces, (obj)->numvtextures);
}

void h3d_obj_loadfile(h3d_obj *obj, char *filename) {
  FILE *f = fopen(filename, "r");
  if (f == NULL) {
    dieerr("Can't open %s for reading object\n", filename);
  }
  h3d_obj_load(obj, f);
  fclose(f);
  eprintf("Read from object file %s\n", filename);
}

// ****************** 3d ************************

// Insert the entirety of an object into another. IT'S UP TO YOU TO KNOW
// IF THE DEST OBJECT HAS ENOUGH SPACE!
void h3d_obj_addobj(h3d_obj *dest, h3d_obj *src, vec3 pos, vec3 lookvec,
                    vec3 up, vec3 scale) {
  // Create model matrix
  mat4 modelm;
  h3d_model_matrix(pos, lookvec, up, scale, modelm);
  // Put all the vertices from the src into the destination after
  // applying transformations to it
  h3d_obj_batchtranslate(src, modelm, dest->vertices + dest->numvertices);
  // Copy over textures + normals
  memcpy(dest->vtexture + dest->numvtextures, src->vtexture,
         sizeof(vec3) * src->numvtextures);
  memcpy(dest->vnormals + dest->numvnormals, src->vnormals,
         sizeof(vec3) * src->numvnormals);
  // Create new faces by assigning indices to the newly copied verts, textures,
  // and normals
  for (int i = 0; i < src->numfaces; i++) {
    for (int vi = 0; vi < 3; vi++) {
      dest->faces[dest->numfaces][vi].verti =
          src->faces[i][vi].verti + dest->numvertices;
      dest->faces[dest->numfaces][vi].texi =
          src->faces[i][vi].texi + dest->numvtextures;
      dest->faces[dest->numfaces][vi].normi =
          src->faces[i][vi].normi + dest->numvnormals;
    }
    dest->numfaces++;
  }
  dest->numvertices += src->numvertices;
  dest->numvtextures += src->numvtextures;
  dest->numvnormals += src->numvnormals;
}

// Batch convert all vertices in an object into translated homogenous vertices.
// This is a very common operation done for triangle rendering
int h3d_obj_batchtranslate(h3d_obj *object, mat4 matrix, vec4 *out) {
  for (int i = 0; i < object->numvertices; i++) {
    // This is SLOW but safe. If you want a faster translation, you may
    // want to skip the homogenous conversion
    vec4 tmp;
    memcpy(tmp, object->vertices[i], sizeof(vec4));
    h3d_vec4_homogenous_real(tmp);
    h3d_vec4_mult_mat4(tmp, matrix, out[i]);
  }
  return object->numvertices;
}

// ========================================
// |            3DFACE                    |
// ========================================

// Determine an "intensity" for a face compared against the given light source.
// Does a very simple calculation
hfloat_t h3d_3dface_light(hfloat_t *light, hfloat_t minlight, h3d_3dface face) {
  vec3 lnorm;
  h3d_3dface_normal(face, lnorm);
  hfloat_t intensity =
      light[0] * lnorm[0] + light[1] * lnorm[1] + light[2] * lnorm[2];
  if (intensity < minlight) {
    return minlight; // Don't just not draw the triangle: it should be black
  } else {
    return (intensity + minlight) / (H3DVF(1) + minlight);
  }
}
