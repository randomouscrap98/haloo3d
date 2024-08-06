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

#endif
