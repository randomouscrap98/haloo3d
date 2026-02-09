#include "collision.h"

#include <float.h>

void collision_objface_aabb(h3d_obj * model, uint32_t face_idx, vec3 min, vec3 max) {
  VEC3(min, FLT_MAX, FLT_MAX, FLT_MAX);
  VEC3(max, FLT_MIN, FLT_MIN, FLT_MIN);
  //printf("face_idx %d\n", face_idx);
  for(int vi = 0; vi < 3; vi++) { // for each vertex
    for(int pi = 0; pi < 3; pi++) { // for each dimension in this vertex
      // The min/max for this dimension is the min of any of the vertices
      min[pi] = H3D_MIN(min[pi], model->vertices[model->faces[face_idx][vi].verti][pi]);
      max[pi] = H3D_MAX(max[pi], model->vertices[model->faces[face_idx][vi].verti][pi]);
    }
  }
}
