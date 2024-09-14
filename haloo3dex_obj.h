#ifndef HALOO3D_OBJ_H
#define HALOO3D_OBJ_H

#include "haloo3d.h"
#include <stdio.h>

int haloo3d_obj_addvertex(haloo3d_obj *obj, struct vec4 vertex);
int haloo3d_obj_addvtexture(haloo3d_obj *obj, struct vec3 vtexture);
int haloo3d_obj_addface(haloo3d_obj *obj, haloo3d_facei face);

// Parse a single obj file into the given obj object.
// Make sure obj is not currently in use (memory leaks).
// This function may not parse all obj files, and certainly
// doesn't parse every feature
void haloo3d_obj_load(haloo3d_obj *obj, FILE *f);
// Free the data created by haloo3d_obj_load
void haloo3d_obj_free(haloo3d_obj *obj);
// Load an object from a file. If it fails, it kills the whole program
void haloo3d_obj_loadfile(haloo3d_obj *obj, char *filename);
// Load an object from a string. If it fails, it kill sthe whole program
void haloo3d_obj_loadstring(haloo3d_obj *obj, const char *str);
// Parse a single line from some object file, modifying the data in obj
void haloo3d_obj_parseline(haloo3d_obj *obj, char *line);

// Reset some model to all 0 counts, and malloc some faces and vertices
// according to given numbers
void haloo3d_obj_resetfixed(haloo3d_obj *obj, int faces, int vertices);
// If an object was over-malloced, this will shrink it to fit its new bounds.
// You should only do this once your model is set in stone.
void haloo3d_obj_shrinktofit(haloo3d_obj *obj);

#endif
