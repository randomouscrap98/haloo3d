#include "collision.h"

#include <float.h>

void collision_box_3d_init(collision_box_3d * box) {
  VEC3(box->min, 0, 0, 0);
  VEC3(box->max, 0, 0, 0);
}

void collision_objface_aabb(h3d_obj * model, uint32_t face_idx, collision_box_3d * box) {
  VEC3(box->min, FLT_MAX, FLT_MAX, FLT_MAX);
  VEC3(box->max, -FLT_MAX, -FLT_MAX, -FLT_MAX);
  //printf("face_idx %d\n", face_idx);
  for(int vi = 0; vi < 3; vi++) { // for each vertex
    for(int pi = 0; pi < 3; pi++) { // for each dimension in this vertex
      // The min/max for this dimension is the min of any of the vertices
      box->min[pi] = H3D_MIN(box->min[pi], model->vertices[model->faces[face_idx][vi].verti][pi]);
      box->max[pi] = H3D_MAX(box->max[pi], model->vertices[model->faces[face_idx][vi].verti][pi]);
    }
  }
}
