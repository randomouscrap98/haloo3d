// Functions for generating textures, models, etc

#ifndef HALOO3D_GEN_H
#define HALOO3D_GEN_H

#include "haloo3d.h"
#include <stdint.h>
#include <stdio.h>

// Generate a checkerboard pattern within the texture
void haloo3d_gen_checkerboard(haloo3d_fb *fb, uint16_t *cols, uint16_t numcols,
                              uint16_t size);
// Generate a 1 pixel wide gradient from bottom to top
void haloo3d_gen_1pxgradient(haloo3d_fb *fb, uint16_t bottom, uint16_t top,
                             uint16_t height);
// Generate a cube where all faces face inwards
void haloo3d_gen_skybox(haloo3d_obj *obj);
// Generate a flat plane made up of individual squares
void haloo3d_gen_plane(haloo3d_obj *obj, uint16_t size);

// Generate a simple mountain where the center is the top.
// This one expects your obj to already be initialized
void haloo3d_gen_sloped(haloo3d_obj *obj, uint16_t size, mfloat_t slopiness,
                        mfloat_t downbias);

#endif
