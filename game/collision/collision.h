#ifndef __COLLISION_COLLISION_H__
#define __COLLISION_COLLISION_H__

#include "../../haloo3d_obj.h"

typedef struct {
  vec3 min;     // Position of calculated AABB box
  vec3 max;     // Position of calculated AABB box
} collision_box_3d;

void collision_box_3d_init(collision_box_3d * box);

// Calculate AABB from face in given model
void collision_objface_aabb(h3d_obj * model, uint32_t face_idx, collision_box_3d * out);

// Calculate AABB collision with a min and max position forming a rect
#define AABBCOLLIDE_XY(x1, y1, x1m, y1m, x2, y2, x2m, y2m) \
  (((x1) < (x2m)) && ((y1) < (y2m)) && \
   ((x2) < (x1m)) && ((y2) < (y1m)))

// Calculate AABB collision with a min and max position forming a cube
#define AABBCOLLIDE_XYZ(x1, y1, z1, x1m, y1m, z1m, x2, y2, z2, x2m, y2m, z2m) \
  (((x1) < (x2m)) && ((y1) < (y2m)) && ((z1) < (z2m)) && \
   ((x2) < (x1m)) && ((y2) < (y1m)) && ((z2) < (z1m)))

// Calculate AABB collision with a position and dim (in positive direction) forming rect
#define AABBCOLLIDE_2DDIM(p1, d1, p2, d2) \
  AABBCOLLIDE_XY( \
    p1[0], p1[1], p1[0] + d1[0], p1[1] + d1[1], \
    p2[0], p2[1], p2[0] + d2[0], p2[1] + d2[1] \
  )

// Calculate AABB collision with a position and dim (in positive direction) forming cube
#define AABBCOLLIDE_3DDIM(p1, d1, p2, d2) \
  AABBCOLLIDE_XYZ( \
    p1[0], p1[1], p1[2], p1[0] + d1[0], p1[1] + d1[1], p1[2] + d1[2], \
    p2[0], p2[1], p2[2], p2[0] + d2[0], p2[1] + d2[1], p2[2] + d2[2] \
  )

// Calculate AABB collision with a min position and max position (in positive direction) forming rect
#define AABBCOLLIDE_2DMINMAX(p1, p1m, p2, p2m) \
  AABBCOLLIDE_XY( \
    p1[0], p1[1], p1m[0], p1m[1], \
    p2[0], p2[1], p2m[0], p2m[1] \
  )

// Calculate AABB collision with a min position and max position (in positive direction) forming cube
#define AABBCOLLIDE_3DMINMAX(p1, p1m, p2, p2m) \
  AABBCOLLIDE_XYZ( \
    p1[0], p1[1], p1[2], p1m[0], p1m[1], p1m[2], \
    p2[0], p2[1], p2[2], p2m[0], p2m[1], p2m[2] \
  )

#define AABBCOLLIDE_3DBOX(p1, p2) \
  AABBCOLLIDE_XYZ( \
    (p1)->min[0], (p1)->min[1], (p1)->min[2], (p1)->max[0], (p1)->max[1], (p1)->max[2], \
    (p2)->min[0], (p2)->min[1], (p2)->min[2], (p2)->max[0], (p2)->max[1], (p2)->max[2] \
  )

#endif
