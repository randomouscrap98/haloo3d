#include "object.h"
// #include "triangles.h"
#include "../../haloo3d_ex.h"
#include "../utils/utils.h"

void render_vert_cache_init(render_vert_cache * rvc) {
  rvc->length = 0;
  rvc->vertices = NULL;
}

void render_vert_cache_free(render_vert_cache * rvc) {
  NULLFREE(rvc->vertices);
}

void render_vert_cache_accommodate(render_vert_cache * rvc, const h3d_obj * obj) {
  size_t new_length = (obj->numvertices + VERTEX_PRECALC_EXTEND);
  size_t new_size = sizeof(vec4) * new_length;
  if(rvc->vertices == NULL) {
    rvc->length = new_length;
    mallocordie(rvc->vertices, new_size);
  } else if (rvc->length < obj->numvertices) {
    rvc->length = new_length;
    reallocordie(rvc->vertices, new_size);
  }
}

void h3d_obj_make_3dface(const h3d_obj *_obj, const vec4 *verttrans,
                         uint16_t fi, h3d_3dface out) { //, uint8_t douv) {
  for (int v = 0; v < 3; v++) {
    // Use the translated vertices instead of the object's 
    memcpy(out[v].pos, verttrans[_obj->faces[fi][v].verti], sizeof(vec4));
    // Setup the interpolants as though we're going to use them directly
    // (we don't know if the triangle draw will use 1/z etc, let it decide)
    out[v].interpolants[0] = out[v].pos[H3DW];
    out[v].interpolants[1] = _obj->vtexture[_obj->faces[fi][v].texi][H3DX];
    out[v].interpolants[2] = _obj->vtexture[_obj->faces[fi][v].texi][H3DY];
  }
  //return _obj->faces[fi][0].texi;
}

// void h3d_obj_render(h3d_obj * obj, mat4 full_transform, render_vert_cache * rvc) {
//   render_vert_cache_accommodate(rvc, obj);
//   h3d_obj_batchtranslate(obj, full_transform, rvc->vertices);
// 
//   h3d_3dface face, baseface;
//   h3d_3dface outfaces[H3D_MAXCLIP];
//   // h3d_rasterface drawface;
//   for (uint32_t facei = 0; facei < obj->numfaces; facei++) {
//     // Copy face values out of precalc array and clip them
//     h3d_obj_make_3dface(obj, rvc->vertices, facei, face);
//     int tris = h3d_3dface_clip(face, outfaces, 3);
//     if (tris > 0) { // If statement in case there's lighting/etc
//       for (int ti = 0; ti < tris; ti++) {
//         int backface = !h3d_3dface_normalize(outfaces[ti]);
//         if (o->cullbackface && backface) {
//           continue;
//         }
//         triangle_maze(h3d_fb *fb, outfaces[ti], const triangle_properties * t,
//                const render_environment * r);
//         //(*ecs)->totaldrawn++;
//         //render_(&o->context[ctx]->window, outfaces[ti], o->texture,
//                   //o->flags & OBJFLAG_ONECOLOR ? col : 0);
//       }
//     }
//   }
// }
