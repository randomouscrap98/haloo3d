#include "haloo3dex_obj.h"
#include "haloo3d.h"

#include <stdlib.h>
#include <string.h>

#define H3D_OBJ_MAXLINESIZE 1024

int haloo3d_obj_addvertex(haloo3d_obj *obj, struct vec4 vertex) {
  int vi = obj->numvertices++;
  obj->vertices[vi] = vertex;
  return vi;
}

int haloo3d_obj_addvtexture(haloo3d_obj *obj, struct vec3 vtexture) {
  int vti = obj->numvtextures++;
  obj->vtexture[vti] = vtexture;
  return vti;
}

int haloo3d_obj_addface(haloo3d_obj *obj, haloo3d_facei face) {
  int fi = obj->numfaces++;
  memcpy(&obj->faces[fi], face, sizeof(haloo3d_facei));
  return fi;
}

// Initialize an object with all 0 amounts BUT allocate all arrays to
// the indicated size. This makes it ready for loading
void haloo3d_obj_resetfixed(haloo3d_obj *obj, int faces, int vertices) {
  obj->numfaces = 0;
  obj->numvertices = 0;
  obj->numvtextures = 0;
  obj->numvnormals = 0;

  // NOTE: to make life easier. this allocates a LOT of memory
  // to start with! It then frees it after so the obj is as small as possible.
  mallocordie(obj->faces, sizeof(haloo3d_facei) * faces);
  mallocordie(obj->vertices, sizeof(struct vec4) * vertices);
  mallocordie(obj->vtexture, sizeof(struct vec3) * vertices);
}

static inline void haloo3d_obj_resetmax(haloo3d_obj *obj) {
  haloo3d_obj_resetfixed(obj, H3D_OBJ_MAXFACES, H3D_OBJ_MAXVERTICES);
}

// Resize all arrays so they're exactly the size needed for the numbers
// in the given obj
void haloo3d_obj_shrinktofit(haloo3d_obj *obj) {
  reallocordie(obj->faces, sizeof(haloo3d_facei) * MAX(1, obj->numfaces));
  reallocordie(obj->vertices, sizeof(struct vec4) * MAX(1, obj->numvertices));
  reallocordie(obj->vtexture, sizeof(struct vec3) * MAX(1, obj->numvtextures));
}

void haloo3d_obj_load(haloo3d_obj *obj, FILE *f) {
  haloo3d_obj_resetmax(obj);
  // Assumes the file pointer is at the point you want it at
  char line[H3D_OBJ_MAXLINESIZE];
  while (fgets(line, H3D_OBJ_MAXLINESIZE, f)) {
    haloo3d_obj_parseline(obj, line);
  }
  haloo3d_obj_shrinktofit(obj);
}

void haloo3d_obj_loadstring(haloo3d_obj *obj, const char *str) {
  haloo3d_obj_resetmax(obj);
  char line[H3D_OBJ_MAXLINESIZE];
  char *endline;
  int running = 1;
  while (running) {
    endline = strchr(str, '\n');
    if (endline) {
      strncpy(line, str, MIN(H3D_OBJ_MAXLINESIZE - 1, endline - str));
      str = endline + 1;
    } else {
      strncpy(line, str, MIN(H3D_OBJ_MAXLINESIZE - 1, strlen(str)));
      running = 0;
    }
    haloo3d_obj_parseline(obj, line);
  }
  haloo3d_obj_shrinktofit(obj);
  eprintf("Read from object string: v=%d, f=%d, t=%d\n", obj->numvertices,
          obj->numfaces, obj->numvtextures);
}

void haloo3d_obj_parseline(haloo3d_obj *obj, char *line) {
  char tmp[H3D_OBJ_MAXLINESIZE];
  char *next = line;
  // Read the first sector, we only support some of them
  int scanned = sscanf(next, "%s", tmp);
  if (scanned == 0) { // empty line or something
    return;
  }
  next += strlen(tmp);
  if (strcmp(tmp, "v") == 0) {
    if (obj->numvertices >= H3D_OBJ_MAXVERTICES) {
      eprintf("Too many object vertices (%d)!\n", obj->numvertices);
      return;
    }
    // v can have 3 or 4 floats. We try to read 4 floats
    struct vec4 *v = obj->vertices + obj->numvertices;
    scanned =
        sscanf(next, "%f %f %f %f", (*v).v, (*v).v + 1, (*v).v + 2, (*v).v + 3);
    if (scanned < 4) {
      (*v).w = 1.0; // default value for w
    }
    obj->numvertices++;
  } else if (strcmp(tmp, "vt") == 0) {
    if (obj->numvtextures >= H3D_OBJ_MAXVERTICES) {
      eprintf("Too many object vtexture points (%d)!\n", obj->numvtextures);
      return;
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
      eprintf("Too many object vnormal points (%d)!\n", obj->numvnormals);
      return;
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
    if (obj->numfaces >= H3D_OBJ_MAXFACES) {
      eprintf("Too many object faces! (%d)\n", obj->numfaces);
      return;
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
        if (scanned == 0) {
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
          (ti >= obj->numvtextures && obj->numvtextures) || ni < 0 ||
          (ni >= obj->numvnormals && obj->numvnormals)) {
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

void haloo3d_obj_free(haloo3d_obj *obj) {
  free(obj->faces);
  free(obj->vertices);
  free(obj->vtexture);
}

void haloo3d_obj_loadfile(haloo3d_obj *obj, char *filename) {
  // Open a simple file and read the ppm from it
  FILE *f = fopen(filename, "r");
  if (f == NULL) {
    dieerr("Can't open %s for reading object\n", filename);
  }
  haloo3d_obj_load(obj, f);
  fclose(f);
  eprintf("Read from object file %s\n", filename);
}
