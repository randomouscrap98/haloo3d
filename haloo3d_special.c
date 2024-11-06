#include "haloo3d_special.h"
#include "haloo3d_3d.h"

#include <string.h>

// Insert the entirety of an object into another. IT'S UP TO YOU TO KNOW
// IF THE DEST OBJECT HAS ENOUGH SPACE!
void h3d_obj_addobj(h3d_obj *dest, h3d_obj *src, vec3 pos, vec3 lookvec,
                    vec3 up, vec3 scale) {
  vec4 tmp;
  mat4 modelm;
  vec3_add(pos, lookvec, tmp);
  h3d_my_lookat(pos, tmp, up, modelm);
  // Apply scale such that it looks like it was applied first (this prevents
  // scaling applying skew to a rotated object)
  h3d_mat4_prescale_self(modelm, scale);
  // Put all the vertices from the src into the destination after
  // applying transformations to it
  for (int i = 0; i < src->numvertices; i++) {
    h3d_vec4_mult_mat4(src->vertices[i], modelm,
                       dest->vertices[dest->numvertices + i]);
  }
  memcpy(dest->vtexture + dest->numvtextures, src->vtexture,
         sizeof(vec3) * src->numvtextures);
  memcpy(dest->vnormals + dest->numvnormals, src->vnormals,
         sizeof(vec3) * src->numvnormals);
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
