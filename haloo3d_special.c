#include "haloo3d_special.h"
#include "haloo3d_3d.h"

#include <string.h>

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
