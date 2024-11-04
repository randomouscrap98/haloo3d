#ifndef HALOO3D_OBJ_H
#define HALOO3D_OBJ_H

#include "haloo3d.h"
#include <stdio.h>

#define H3D_OBJ_MAXVERTICES 8192
#define H3D_OBJ_MAXFACES 8192

// A vertex which is made up of indexes into the obj
typedef struct {
  uint16_t posi;
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

int h3d_obj_addvertex(h3d_obj *obj, float_t *vertex);
int h3d_obj_addvtexture(h3d_obj *obj, float_t *vtexture);
int h3d_obj_addface(h3d_obj *obj, h3d_objvert *face);

// Parse a single obj file into the given obj object.
// Make sure obj is not currently in use (memory leaks).
// This function may not parse all obj files, and certainly
// doesn't parse every feature
void h3d_obj_load(h3d_obj *obj, FILE *f);
// Free the data created by haloo3d_obj_load
void h3d_obj_free(h3d_obj *obj);
// Load an object from a file. If it fails, it kills the whole program
void h3d_obj_loadfile(h3d_obj *obj, char *filename);
// Load an object from a string. If it fails, it kill sthe whole program
void h3d_obj_loadstring(h3d_obj *obj, const char *str);
// Parse a single line from some object file, modifying the data in obj
void h3d_obj_parseline(h3d_obj *obj, char *line);

// Reset some model to all 0 counts, and malloc some faces and vertices
// according to given numbers
void h3d_obj_resetfixed(h3d_obj *obj, int faces, int vertices);
// If an object was over-malloced, this will shrink it to fit its new bounds.
// You should only do this once your model is set in stone.
void h3d_obj_shrinktofit(h3d_obj *obj);

// Add an object to another object. The src object is not modified.
// IT IS UP TO YOU TO MAKE SURE THERE'S ENOUGH SPACE FOR THE SRC IN DEST!
// void h3d_obj_addobj(h3d_obj *dest, h3d_obj *src, float_t *pos, float_t
// *lookvec,
//                     float_t *up, float_t *scale);
#endif
