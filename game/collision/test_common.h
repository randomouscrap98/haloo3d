#ifndef __COLLISION_TEST_COMMON_H__
#define __COLLISION_TEST_COMMON_H__

#include "../../haloo3d_obj.h"
#include "../utils/print.h"
#include <assert.h>

// This also serves as our "basic triangle"
static const char onetri[1024] =
  "v -1.0 -1.0 -1.0\n"
  "v 1.0 -1.0 2.0\n"
  "v 0.0 3.0 0.0\n"
  "f 1 2 3\n";

static inline void eighttri(h3d_obj * obj) {
  vec4 objvertex;
  // Now, let's add some more faces and regenerate. We're going to be generous
  // and put JUST ONE triangle in every quadrant
  int vertcount = 0;
  h3d_objface face;
  for(int z = -1; z < 1; z++) {
    for(int y = -1; y < 1; y++) {
      for(int x = -1; x < 1; x++) {
        for(int i = 0; i < 3; i++) {
          VEC4(objvertex, x + 0.1 * i, y + 0.1 * i, z + 0.1 * i, 1.0);
          h3d_obj_addvertex(obj, objvertex);
          face[i].normi = 0;
          face[i].texi = 0;
          face[i].verti = vertcount++;
        }
        h3d_obj_addface(obj, face);
      }
    }
  }

  assert(obj->numfaces == 8 && "eighttri");
  assert(obj->numvertices == 24 && "eighttri");
  for(int fi = 0; fi < (int)obj->numfaces; fi++) { // each face
    printf("eighttri face %d\n", fi);
    for(int vi = 0; vi < 3; vi++) { // each vertex in the face
      printf(VEC3FMT(3)"\n", VEC3SPREAD(obj->vertices[obj->faces[fi][vi].verti]));
    }
  }
}

#endif
