#ifndef __RENDER_OBJECT_H__
#define __RENDER_OBJECT_H__

#include "../../haloo3d.h"
#include "../../haloo3d_obj.h"
#include "../../haloo3d_3d.h"

// A linear amount of extra space to extend the precalc buffer by
// if it's too small to house the next object
#define VERTEX_PRECALC_EXTEND 1024

typedef struct {
  vec4 * vertices;
  size_t length;
} render_vert_cache;

void render_vert_cache_init(render_vert_cache * rvc);
void render_vert_cache_free(render_vert_cache * rvc);
void render_vert_cache_accommodate(render_vert_cache * rvc, const h3d_obj * obj);

void h3d_obj_make_3dface(const h3d_obj *_obj, const vec4 *verttrans,
                         uint16_t fi, h3d_3dface out);

#endif
