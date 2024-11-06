#include "haloo3d_3d.h"

void h3d_my_lookat(float_t *from, float_t *to, float_t *up, float_t *view) {
  vec3 forward;
  vec3 right;
  vec3 realup;
  vec3_subtract(from, to, forward);
  vec3_normalize(forward, forward);
  vec3_cross(up, forward, right);
  vec3_normalize(right, right); // IDK if you have to normalize but whatever
  vec3_cross(forward, right, realup);

  mat4_identity(view);
  view[0] = right[H3DX];
  view[1] = right[H3DY];
  view[2] = right[H3DZ];
  view[4] = realup[H3DX];
  view[5] = realup[H3DY];
  view[6] = realup[H3DZ];
  view[8] = forward[H3DX];
  view[9] = forward[H3DY];
  view[10] = forward[H3DZ];
  view[12] = from[H3DZ];
  view[13] = from[H3DY];
  view[14] = from[H3DZ];
  // Remember: I don't pre-invert it. That wastes slightly more time but it
  // lets this function be more useful I think...
}

// My personal perspective matrix setter which uses horizontal fov
// and aspect ratio
void h3d_perspective(float_t fov, float_t aspect, float_t near, float_t far,
                     float_t *m) {
  mat4_zero(m);

  fov = fov / H3DVF(180) * H3DVF(MPI); // Convert to radians
  float_t e = H3DVF(1.0) / tanf(fov * H3DVF(0.5));

  m[0] = e / aspect;
  m[5] = e;
  m[10] = (far + near) / (near - far);
  m[11] = H3DVF(-1); // the z divide
  m[14] = H3DVF(2) * far * near / (near - far);
}
