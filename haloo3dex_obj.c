#include "haloo3dex_obj.h"
#include "haloo3d.h"

#include <stdlib.h>

#define H3D_OBJ_MAXLINESIZE 1024

void haloo3d_obj_load(haloo3d_obj *obj, FILE *f) {
  obj->numfaces = 0;
  obj->numvertices = 0;
  obj->numvtextures = 0;
  // NOTE: to make life easier. this allocates a LOT of memory
  // to start with! It then frees it after so the obj is as small as possible.
  mallocordie(obj->faces, sizeof(haloo3d_facei) * H3D_OBJ_MAXFACES);
  mallocordie(obj->vertices, sizeof(vec4f) * H3D_OBJ_MAXVERTICES);
  mallocordie(obj->vtexture, sizeof(vec3f) * H3D_OBJ_MAXVERTICES);

  // Assumes the file pointer is at the point you want it at
  // size_t read;
  char line[H3D_OBJ_MAXLINESIZE];
  // size_t len = 0;

  while (fgets(line, H3D_OBJ_MAXLINESIZE, f)) {
    printf("%s\n", line);
  }
  // IDK I got this from the internet.
  // while ((read = getline(&line, &len, f)) != -1) {
  //   printf("Retrieved line of length %zu:\n", read);
  //   printf("%s", line);
  // }

  reallocordie(obj->faces, sizeof(haloo3d_facei) * MIN(1, obj->numfaces));
  reallocordie(obj->vertices, sizeof(vec4f) * MIN(1, obj->numvertices));
  reallocordie(obj->vtexture, sizeof(vec3f) * MIN(1, obj->numvtextures));
}

void haloo3d_obj_free(haloo3d_obj *obj) {
  free(obj->faces);
  free(obj->vertices);
  free(obj->vtexture);
}
