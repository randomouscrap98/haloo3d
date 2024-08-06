#include "haloo3dex_obj.h"
#include "haloo3d.h"
#include "mathc.h"

#include <stdlib.h>
#include <string.h>

#define H3D_OBJ_MAXLINESIZE 1024

void haloo3d_obj_load(haloo3d_obj *obj, FILE *f) {
  obj->numfaces = 0;
  obj->numvertices = 0;
  obj->numvtextures = 0;
  // NOTE: to make life easier. this allocates a LOT of memory
  // to start with! It then frees it after so the obj is as small as possible.
  mallocordie(obj->faces, sizeof(haloo3d_facei) * H3D_OBJ_MAXFACES);
  mallocordie(obj->vertices, sizeof(struct vec4) * H3D_OBJ_MAXVERTICES);
  mallocordie(obj->vtexture, sizeof(struct vec3) * H3D_OBJ_MAXVERTICES);

  // Assumes the file pointer is at the point you want it at
  char line[H3D_OBJ_MAXLINESIZE];
  char tmp[H3D_OBJ_MAXLINESIZE];

  while (fgets(line, H3D_OBJ_MAXLINESIZE, f)) {
    char *next = line;
    // Read the first sector, we only support some of them
    int scanned = sscanf(next, "%s", tmp);
    if (scanned == 0) { // empty line or something
      continue;
    }
    next += strlen(tmp);
    if (strcmp(tmp, "v") == 0) {
      if (obj->numvertices >= H3D_OBJ_MAXVERTICES) {
        eprintf("Too many object vertices!\n");
        continue;
      }
      // v can have 3 or 4 floats. We try to read 4 floats
      struct vec4 *v = obj->vertices + obj->numvertices;
      scanned = sscanf(next, "%f %f %f %f", (*v).v, (*v).v + 1, (*v).v + 2,
                       (*v).v + 3);
      if (scanned < 4) {
        (*v).w = 1.0; // default value for w
      }
      obj->numvertices++;
    } else if (strcmp(tmp, "vt") == 0) {
      if (obj->numvtextures >= H3D_OBJ_MAXVERTICES) {
        eprintf("Too many object vtexture points!\n");
        continue;
      }
      // v can have 1 to 3 floats. Read all, and it's fine if we don't get them
      struct vec3 *v = obj->vtexture + obj->numvtextures;
      scanned = sscanf(next, "%f %f %f", (*v).v, (*v).v + 1, (*v).v + 2);
      if (scanned < 3) {
        (*v).z = 0.0; // default value for z
      }
      if (scanned < 2) {
        (*v).y = 0.0; // default value for y (v)
      }
      obj->numvtextures++;
    } else if (strcmp(tmp, "vn") == 0) {
      if (obj->numvnormals >= H3D_OBJ_MAXVERTICES) {
        eprintf("Too many object vnormal points!\n");
        continue;
      }
      // vn must have 3 floats
      // vec3f *v = obj->vtexture + obj->numvtextures;
      // scanned = sscanf(next, "%f %f %f", (*v), (*v) + 1, (*v) + 2);
      // if (scanned < 3) {
      //   eprintf("Vertex normal %d did not have enough arguments",
      //   obj->numnormals); continue;
      // }
      // For now, only track them. I don't want to waste memory
      obj->numvnormals++;
    } else if (strcmp(tmp, "f") == 0) {
      if (obj->numvnormals >= H3D_OBJ_MAXFACES) {
        eprintf("Too many object faces!\n");
        continue;
      }
      haloo3d_facei *f = obj->faces + obj->numfaces;
      uint8_t err = 0;
      // We must read the 3 vertex information. We'll read them one
      // at a time as a string, then parse them
      for (int i = 0; i < 3; i++) {
        scanned = sscanf(next, "%s", tmp);
        if (scanned < 1) {
          // This is some kinda weird error...
          eprintf("Face %d did not have enough arguments\n", obj->numfaces);
          err = 1;
          break;
        }
        int vi, ti, ni; // the three numbers we're trying to parse
        vi = ti = ni = 1;
        // There are only 4 formats for the face: v, v/t, v/t/n, and v//n.
        // Start with the weird format first.
        scanned = sscanf(tmp, "%d//%d", &vi, &ni);
        if (scanned < 2) { // if we did not specifically read the weird format
          scanned = sscanf(tmp, "%d/%d/%d", &vi, &ti, &ni);
          if (scanned != 1 && scanned != 3) {
            eprintf("Could not parse face: %s | %s\n", tmp, line);
            err = 1;
            break;
          }
        }
        // Whatever was scanned up to this point is fine... we allow 0s
        if (vi < 0) {
          vi += obj->numvertices;
        } else {
          vi -= 1;
        }
        if (ti < 0) {
          ti += obj->numvtextures;
        } else {
          ti -= 1;
        }
        if (ni < 0) {
          ni += obj->numvnormals;
        } else {
          ni -= 1;
        }
        // Check each against the lists to make sure it's not out of whatever
        if (vi < 0 || vi >= obj->numvertices || ti < 0 ||
            ti >= obj->numvtextures || ni < 0 || ni >= obj->numvnormals) {
          eprintf("Invalid face index: %s | %s\n", tmp, line);
          err = 1;
          break;
        }
        // Now we can assign the values
        (*f)[i].posi = vi;
        (*f)[i].texi = ti;
        (*f)[i].normi = ni;
        next = strstr(next, tmp) + strlen(tmp);
      }
      // Don't increase numfaces if there was an error
      if (!err) {
        obj->numfaces++;
      }
    } else {
      // printf("Unrecognized line: %s", line);
    }
  }

  reallocordie(obj->faces, sizeof(haloo3d_facei) * MAX(1, obj->numfaces));
  reallocordie(obj->vertices, sizeof(struct vec4) * MAX(1, obj->numvertices));
  reallocordie(obj->vtexture, sizeof(struct vec3) * MAX(1, obj->numvtextures));
}

void haloo3d_obj_free(haloo3d_obj *obj) {
  free(obj->faces);
  free(obj->vertices);
  free(obj->vtexture);
}
