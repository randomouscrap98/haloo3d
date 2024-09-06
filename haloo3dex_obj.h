#ifndef HALOO3D_OBJ_H
#define HALOO3D_OBJ_H

#include "haloo3d.h"
#include <stdio.h>

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

#endif
