#ifndef __HALOO3D_OBJ_H
#define __HALOO3D_OBJ_H

// Objects and functions for dealing with wavefront obj files.
// The haloo3d_obj struct is often used raw in various other places
// in the library

#include "haloo3d.h"
#include <stdio.h>
#include <string.h>

#define H3D_OBJ_MAXVERTICES 8192
#define H3D_OBJ_MAXFACES 8192
#define H3D_OBJ_MAXLINESIZE 1024

// A vertex which is made up of indexes into the obj
typedef struct {
  uint16_t verti;
  uint16_t texi;
  uint16_t normi;
} h3d_objvert;

typedef h3d_objvert h3d_objface[3];

// An object definition, where every face is a simple
// index into the internal structures
typedef struct {
  uint16_t numvertices;
  uint16_t numvtextures;
  uint16_t numfaces;
  uint16_t numvnormals;
  vec4 *vertices;
  vec3 *vtexture;
  vec3 *vnormals;
  h3d_objface *faces;
} h3d_obj;

static inline int h3d_obj_addvertex(h3d_obj *obj, vec4 vertex) {
  int vi = obj->numvertices++;
  memcpy(obj->vertices[vi], vertex, sizeof(vec4));
  return vi;
}

static inline int h3d_obj_addvtexture(h3d_obj *obj, vec3 vtexture) {
  int vti = obj->numvtextures++;
  memcpy(obj->vtexture[vti], vtexture, sizeof(vec3));
  return vti;
}

static int h3d_obj_addface(h3d_obj *obj, h3d_objface face) {
  int fi = obj->numfaces++;
  memcpy(&obj->faces[fi], face, sizeof(h3d_objface));
  return fi;
}

// Parse a single line of wavefront obj file (though not particularly well, mind
// you). We only support v, vt, vn, f
static inline int h3d_obj_parseline(h3d_obj *obj, char *line, char *errout,
                                    int errlen) {
  char tmp[H3D_OBJ_MAXLINESIZE];
  char *next = line;
  // Read the first sector, we only support some of them
  int scanned = sscanf(next, "%s", tmp);
  if (scanned == 0) { // empty line or something
    return 0;
  }
  next += strlen(tmp);
  if (strcmp(tmp, "v") == 0) {
    if (obj->numvertices >= H3D_OBJ_MAXVERTICES) {
      snprintf(errout, errlen, "Too many object vertices (%d)!\n",
               obj->numvertices);
      return 1;
    }
    // v can have 3 or 4 floats. We try to read 4 floats
    float_t *v = (float_t *)(obj->vertices + obj->numvertices);
    scanned = sscanf(next, "%f %f %f %f", v, v + 1, v + 2, v + 3);
    if (scanned < 4) {
      v[H3DW] = 1.0; // default value for w
    }
    obj->numvertices++;
  } else if (strcmp(tmp, "vt") == 0) {
    if (obj->numvtextures >= H3D_OBJ_MAXVERTICES) {
      snprintf(errout, errlen, "Too many object vtexture points (%d)!\n",
               obj->numvtextures);
      return 2;
    }
    // v can have 1 to 3 floats. Read all, and it's fine if we don't get them
    float_t *v = (float_t *)(obj->vtexture + obj->numvtextures);
    scanned = sscanf(next, "%f %f %f", v, v + 1, v + 2);
    if (scanned < 3) {
      v[H3DZ] = 0.0; // default value for z
    }
    if (scanned < 2) {
      v[H3DY] = 0.0; // default value for y (v)
    }
    obj->numvtextures++;
  } else if (strcmp(tmp, "vn") == 0) {
    if (obj->numvnormals >= H3D_OBJ_MAXVERTICES) {
      snprintf(errout, errlen, "Too many object vnormal points (%d)!\n",
               obj->numvnormals);
      return 3;
    }
    // vn must have 3 floats
    float_t *v = (float_t *)(obj->vnormals + obj->numvnormals);
    scanned = sscanf(next, "%f %f %f", v, v + 1, v + 2);
    if (scanned < 3) {
      snprintf(errout, errlen, "Vertex normal %d did not have enough arguments",
               obj->numvnormals);
      return 4;
    }
    obj->numvnormals++;
  } else if (strcmp(tmp, "f") == 0) {
    if (obj->numfaces >= H3D_OBJ_MAXFACES) {
      snprintf(errout, errlen, "Too many object faces! (%d)\n", obj->numfaces);
      return 5;
    }
    h3d_objvert *f = (h3d_objvert *)(obj->faces + obj->numfaces);
    uint8_t err = 0;
    // We must read the 3 vertex information. We'll read them one
    // at a time as a string, then parse them
    for (int i = 0; i < 3; i++) {
      scanned = sscanf(next, "%s", tmp);
      if (scanned < 1) {
        // This is some kinda weird error...
        snprintf(errout, errlen, "Face %d did not have enough arguments\n",
                 obj->numfaces);
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
          snprintf(errout, errlen, "Could not parse face: %s | %s\n", tmp,
                   line);
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
        snprintf(errout, errlen, "Invalid face index: %s | %s\n", tmp, line);
        err = 1;
        break;
      }
      // Now we can assign the values
      f[i].verti = vi;
      f[i].texi = ti;
      f[i].normi = ni;
      next = strstr(next, tmp) + strlen(tmp);
    }
    // Don't increase numfaces if there was an error
    if (!err) {
      obj->numfaces++;
    } else {
      return 99;
    }
  }
  // printf("Unrecognized line: %s", line);
  return 0;
}

#endif
