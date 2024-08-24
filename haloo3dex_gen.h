// Functions for generating textures, models, etc

#ifndef HALOO3D_GEN_H
#define HALOO3D_GEN_H

#include "haloo3d.h"
#include <stdint.h>
#include <stdio.h>

// Given an ALREADY allocated texture, blend a checkerboard on top of it. You
// can use this to generate solid colors by passing in only one col
void haloo3d_apply_alternating(haloo3d_fb *fb, uint16_t *cols,
                               uint16_t numcols);
// Given an ALREADY allocated texture, blend a vertical gradient on top of it.
// THe
void haloo3d_apply_vgradient(haloo3d_fb *fb, uint16_t top, uint16_t bottom);
// Given an ALREADY allocated texture, blend noise on top of it. if null is
// passed, simple noise is applied. The last value indicates how much of an
// effect the noise has. 1.0 means full application of noise and should
// probably not be used. THe noise should be -1 to 1
void haloo3d_apply_noise(haloo3d_fb *fb, float *noise, float scale);
// Given an ALREADY allocated texture, blend a brick grid on top.
void haloo3d_apply_brick(haloo3d_fb *fb, uint16_t width, uint16_t height,
                         uint16_t color);
// Draw a rectangle of the given width into the given texture. Not guaranteed
// to be fast
void haloo3d_apply_rect(haloo3d_fb *fb, haloo3d_recti rect, uint16_t color,
                        int width);
// Draw a filled rectangle with dithering. The dithering 0 bit is full
// transparency
void haloo3d_apply_fillrect(haloo3d_fb *fb, haloo3d_recti rect, uint16_t color,
                            uint8_t dithering[8]);
// Create a NEW texture that is entirely a solid color. Underneath, this will
// use a tiny 1x1 texture, that you'll still unfortunately need to free.
void haloo3d_gen_solidtex(haloo3d_fb *fb, uint16_t color);

// If you know the amount of vertices, textures, and faces ahead of time, you
// can call this function to easily set all the counters and malloc
void haloo3d_gen_obj_prealloc(haloo3d_obj *obj, uint16_t numverts,
                              uint16_t numvtex, uint16_t numfaces);

// With an already-allocated array for vtextures, this fills it with the
// obvious 4 corners. The order is bottomleft, topleft, bottomright, topright
void haloo3d_gen_boxvtexture(struct vec3 *textures);

// Generate a cube where all faces face inwards
void haloo3d_gen_skybox(haloo3d_obj *obj);
// Generate a flat plane made up of individual squares
void haloo3d_gen_plane(haloo3d_obj *obj, uint16_t size);
// Generate a grid that lines up with a plane. Pass 1 to faces to pregenerate
// full grid of faces (vertices are always available)
void haloo3d_gen_grid(haloo3d_obj *obj, uint16_t size, uint8_t faces);
// Generate a face at the given cell in the given direction. WILL regenerate
// duplicate faces if you pass the same parameters (be careful)
void haloo3d_gen_grid_quad(haloo3d_obj *obj, int x, int y, struct vec2i dir);

// Generate a simple mountain where the center is the top.
// This one expects your obj to already be initialized
void haloo3d_gen_sloped(haloo3d_obj *obj, uint16_t size, mfloat_t slopiness,
                        mfloat_t downbias);

// Generate two perpendicular quads each the same size centered in
// the middle of -1 to 1 in each direction. The exact shape will
// depend on the given texture dimensions
void haloo3d_gen_crossquad(haloo3d_obj *obj, haloo3d_fb *fb,
                           struct vec3 center);

// Generate a single quad aligned on the x axis facing in the
// positive Z direction.
void haloo3d_gen_quad(haloo3d_obj *obj, haloo3d_fb *fb, struct vec3 center);

#endif
