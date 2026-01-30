#ifndef __RENDER_TRIANGLES_H__
#define __RENDER_TRIANGLES_H__

#include "../../haloo3d.h"
#include "../../haloo3d_ex.h"
#include "string.h"

// Fix a rasterface for use in perspective correct things. Expect that first
// interpolant is z, second and third are texture u and v
#define PCORRECT_TEXTURE_INTERPOLANT(rf)                                      \
  /* Persp correct is inverse z */                                            \
  for (int f = 0; f < 3; f++) {                                               \
    rf[f].interpolants[0] = 1.0f / rf[f].interpolants[0];                      \
    rf[f].interpolants[1] *= rf[f].interpolants[0];                           \
    rf[f].interpolants[2] *= rf[f].interpolants[0];                           \
  }

// Convert 3dface into render face for perspective correct texture functions.
// MUST have z, u, and v as first 3 interpolants. The rest... uh?
#define PCORRECT_FACE_INTERPOLANT(f3d, fb, f)                                 \
  /* Convert 3dface to rasterface for shader */                               \
  for (int v = 0; v < 3; v++) {                                               \
    f[v].interpolants[0] = 1.0f / f3[v].interpolants[0];                      \
    f[v].interpolants[1] = f3[v].interpolants[1] * f[v].interpolants[0];      \
    f[v].interpolants[2] = f3[v].interpolants[2] * f[v].interpolants[0];      \
    h3d_viewport(f3[v].pos, fb->width, fb->height, f[v].pos);                 \
  }

typedef struct {
  hfloat_t dither_start;
  hfloat_t dither_end;
  hfloat_t dither_scale;  // Calculated from dither_start and dither_end
} render_environment;

void render_environment_set_dithering(
    render_environment * r, hfloat_t dither_start, hfloat_t dither_end);

typedef struct {
  h3d_fb * texture;
  uint16_t flat_color;
} triangle_properties;

// Get a reference to a triangle property that's just the texture. This 
// IS NOT THREADSAFE AT ALL!!!
triangle_properties * triangle_properties_basic_texture(h3d_fb * texture);

void triangle_maze(const h3d_fb *fb, h3d_3dface f3, const triangle_properties * t,
               const render_environment * r);

#endif
