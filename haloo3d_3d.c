#include "haloo3d_3d.h"

#include <string.h>

// ----------------------
//   Camera
// ----------------------

void h3d_camera_init(h3d_camera *cam) {
  // Initialize the camera to look in a safe direction with
  // reasonable up/etc. You spawn at the origin
  VEC3(cam->up, 0.0, 1.0, 0.0);
  VEC3(cam->pos, 0.0, 0.0, 0.0);
  cam->yaw = 0;
  cam->pitch = MPI / 2;
}

void h3d_camera_calclook(h3d_camera *cam, mat4 view, vec3 lookvec) {
  vec3 lookat;
  // Use sphere equation to compute lookat vector through the two
  // player-controled angles (pitch and yaw)
  YAWP2VEC(cam->yaw, cam->pitch, lookvec);
  vec3_add(cam->pos, lookvec, lookat);
  h3d_my_lookat(cam->pos, lookat, cam->up, view);
}

void h3d_camera_calcmove_yaw(h3d_camera *cam, vec4 delta) {
  mat4 rot;
  mat4_zero(rot);
  mat4_rotation_y(-cam->yaw, rot);
  vec4 tmp;
  memcpy(tmp, delta, sizeof(vec4));
  h3d_vec4_mult_mat4(tmp, rot, delta);
}

// ----------------------
//   3d rendering
// ----------------------

void h3d_my_lookat(vec3 from, vec3 to, vec3 up, mat4 view) {
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
                     mat4 m) {
  mat4_zero(m);

  fov = fov / H3DVF(180) * H3DVF(MPI); // Convert to radians
  float_t e = H3DVF(1.0) / tanf(fov * H3DVF(0.5));

  m[0] = e / aspect;
  m[5] = e;
  m[10] = (far + near) / (near - far);
  m[11] = H3DVF(-1); // the z divide
  m[14] = H3DVF(2) * far * near / (near - far);
}
