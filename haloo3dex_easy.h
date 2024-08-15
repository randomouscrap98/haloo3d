// A bunch of helper functions for common tasks. They may not be
// highly configurable; use the core library functions if you need that
#ifndef __HALOO3D_EASY_H
#define __HALOO3D_EASY_H
#include "haloo3d.h"

// Calculate the dither used for a face when your dither distance starts at
// start and ends at end. Also sets the dither on the render settings.
void haloo3d_easy_calcdither4x4(haloo3d_trirender *settings, haloo3d_facef face,
                                mfloat_t ditherstart, mfloat_t ditherend);

#endif
