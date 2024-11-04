// haloopdy 2024
// An extra header you can use if you want access to image-related
// functions.

#ifndef HALOO3D_IMAGE_H
#define HALOO3D_IMAGE_H

#include "haloo3d.h"
#include <stdio.h>

// Writes a P6 binary ppm from the framebuffer
void haloo3d_img_writeppm(haloo3d_fb *fb, FILE *f);
// Loads a P6 binary ppm into a framebuffer
void haloo3d_img_loadppm(FILE *f, haloo3d_fb *fb);

// Convert given color to transparent in framebuffer
void haloo3d_img_totransparent(haloo3d_fb *fb, uint16_t col);

// Write a P6 binary ppm to a file. Kills whole program if it can't
void haloo3d_img_writeppmfile(haloo3d_fb *fb, char *filename);
// Load a P6 binary ppm into the given texture. Kills whole program
// if it can't.
void haloo3d_img_loadppmfile(haloo3d_fb *tex, char *filename);

#endif
