#include "haloo3d_ex.h"

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

void h3d_obj_loadstring(h3d_obj *obj, char *str) {
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
      strncpy(line, str, H3D_MIN(H3D_OBJ_MAXLINESIZE - 1, strlen(str)));
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
    h3d_vec4_homogenous(tmp);
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
