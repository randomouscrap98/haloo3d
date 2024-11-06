#ifndef __HALOO3D_MATH_H
#define __HALOO3D_MATH_H

#include "haloo3d.h"
#include <math.h>

#define MPI 3.1415926536f

// NOTE: pitch = 0 means pointing straight up, this is to prevent gimbal
// lock!
#define YAWP2VEC(yaw, pitch, out)                                              \
  VEC3(out, sinf(pitch) * sinf(yaw), cosf(pitch), -sinf(pitch) * cosf(yaw));

// ----------------------
//   Math
// ----------------------

// Subtract v1 from v0, storing result in out. Out vector can be either
// of the input vectors safely.
static inline void vec3_subtract(vec3 v0, vec3 v1, vec3 out) {
  out[0] = v0[0] - v1[0];
  out[1] = v0[1] - v1[1];
  out[2] = v0[2] - v1[2];
}

// Add v0 to v1, storing result in out. Out vector can be either
// of the input vectors safely.
static inline void vec3_add(vec3 v0, vec3 v1, vec3 out) {
  out[0] = v0[0] + v1[0];
  out[1] = v0[1] + v1[1];
  out[2] = v0[2] + v1[2];
}

// Normalize v0, storing result in out. Out vector can be input vector
static inline void vec3_normalize(vec3 v0, vec3 out) {
  float_t l = sqrtf(v0[0] * v0[0] + v0[1] * v0[1] + v0[2] * v0[2]);
  out[0] = v0[0] / l;
  out[1] = v0[1] / l;
  out[2] = v0[2] / l;
}

// Compute the cross product of v0 and v1, storing result in out. Out
// CANNOT be either input!
static inline void vec3_cross(vec3 v0, vec3 v1, vec3 out) {
  out[0] = v0[1] * v1[2] - v0[2] * v1[1];
  out[1] = v0[2] * v1[0] - v0[0] * v1[2];
  out[2] = v0[0] * v1[1] - v0[1] * v1[0];
}

// Zero out entire mat4 matrix
static inline void mat4_zero(mat4 result) {
  result[0] = H3DVF(0.0);
  result[1] = H3DVF(0.0);
  result[2] = H3DVF(0.0);
  result[3] = H3DVF(0.0);
  result[4] = H3DVF(0.0);
  result[5] = H3DVF(0.0);
  result[6] = H3DVF(0.0);
  result[7] = H3DVF(0.0);
  result[8] = H3DVF(0.0);
  result[9] = H3DVF(0.0);
  result[10] = H3DVF(0.0);
  result[11] = H3DVF(0.0);
  result[12] = H3DVF(0.0);
  result[13] = H3DVF(0.0);
  result[14] = H3DVF(0.0);
  result[15] = H3DVF(0.0);
}

// Store identity matrix inside given result.
static inline void mat4_identity(mat4 result) {
  result[0] = H3DVF(1.0);
  result[1] = H3DVF(0.0);
  result[2] = H3DVF(0.0);
  result[3] = H3DVF(0.0);
  result[4] = H3DVF(0.0);
  result[5] = H3DVF(1.0);
  result[6] = H3DVF(0.0);
  result[7] = H3DVF(0.0);
  result[8] = H3DVF(0.0);
  result[9] = H3DVF(0.0);
  result[10] = H3DVF(1.0);
  result[11] = H3DVF(0.0);
  result[12] = H3DVF(0.0);
  result[13] = H3DVF(0.0);
  result[14] = H3DVF(0.0);
  result[15] = H3DVF(1.0);
}

// Assign x rotation directly to result
static inline void mat4_rotation_x(float_t f, mat4 result) {
  float_t c = cosf(f);
  float_t s = sinf(f);
  result[5] = c;
  result[6] = s;
  result[9] = -s;
  result[10] = c;
}

// Assign y rotation directly to result
static inline void mat4_rotation_y(float_t f, mat4 result) {
  float_t c = cosf(f);
  float_t s = sinf(f);
  result[0] = c;
  result[2] = -s;
  result[8] = s;
  result[10] = c;
}

// Assign z rotation directly to result
static inline void mat4_rotation_z(float_t f, mat4 result) {
  float_t c = cosf(f);
  float_t s = sinf(f);
  result[0] = c;
  result[1] = s;
  result[4] = -s;
  result[5] = c;
}

// Multiply a vec4 vector by a matrix, transforming it and storing it into the
// given out vector. All the sources should be 4 wide, and w is assumed to be 1.
// The out vector CANNOT be the input vector!
static inline void h3d_vec4_mult_mat4(vec4 v, mat4 m, vec4 out) {
  out[H3DX] = v[H3DX] * m[0] + v[H3DY] * m[4] + v[H3DZ] * m[8] + m[12];
  out[H3DY] = v[H3DX] * m[1] + v[H3DY] * m[5] + v[H3DZ] * m[9] + m[13];
  out[H3DZ] = v[H3DX] * m[2] + v[H3DY] * m[6] + v[H3DZ] * m[10] + m[14];
  out[H3DW] = v[H3DX] * m[3] + v[H3DY] * m[7] + v[H3DZ] * m[11] + m[15];
}

// Apply a "prescale" to some model matrix. This will apply a scale to the
// model-view matrix like you expect, this IS the function you want.
// The scale is assumed to be at least a vec3, scaling in the x, y, and z
static inline void h3d_mat4_prescale_self(mat4 m, vec3 scale) {
  m[0] *= scale[0];
  m[1] *= scale[0];
  m[2] *= scale[0];
  m[3] *= scale[0];
  m[4] *= scale[1];
  m[5] *= scale[1];
  m[6] *= scale[1];
  m[7] *= scale[1];
  m[8] *= scale[2];
  m[9] *= scale[2];
  m[10] *= scale[2];
  m[11] *= scale[2];
  // W in the scale matrix is 1, like identity. No need to do anything
}

// Divide x, y, and z by the w value. Preserves the original w value!!
// This is often the last step of perspective projection (perspective divide)
static inline void h3d_vec4_homogenous(vec4 v) {
  if (v[H3DW] != H3DVF(1)) {
    float_t div = H3DVF(1) / v[H3DW];
    v[H3DX] *= div;
    v[H3DY] *= div;
    v[H3DZ] *= div;
  }
}

// ----------------------
//   Camera
// ----------------------

// Describes a camera which can rotate in two directions only (no rolling)
typedef struct {
  vec3 pos;
  vec3 up;
  float_t pitch;
  float_t yaw;
} h3d_camera;

// Initialize the camera to look in a safe direction with
// reasonable up/etc. You spawn at the origin
void h3d_camera_init(h3d_camera *cam);

// Calculate the look vector (last param), using it to set the given matrix view
// to the "look_at" matrix
void h3d_camera_calclook(h3d_camera *cam, mat4 view, vec3 lookvec);

// Transform given vector by yaw rotation indicated in camera.
// TODO: this function might be unnecessary bloat
void h3d_camera_calcmove_yaw(h3d_camera *cam, vec4 delta);

// ----------------------
//   3d rendering
// ----------------------

// My personal lookat function, which does not perform the inverse or anything.
// Because of this, you can use it to orient models too
void h3d_my_lookat(vec3 from, vec3 to, vec3 up, mat4 view);

// My personal perspective projection function. For some reason, it produces
// different results than the mathc libary's
void h3d_perspective(float_t fov, float_t aspect, float_t near, float_t far,
                     float_t *m);

// Calculate triangle viewport pixel coordinates from normalized coordinates
static inline void h3d_viewport(float_t *v, int width, int height,
                                uint16_t *out) {
  // I'm honestly not sure if round, ceil, or floor is more appropriate.
  // I've read those articles from Chris Hecker indicating that ceil
  // should be used, but it didn't seem to produce the right results.
  out[H3DX] = round((v[H3DX] + H3DVF(1.0)) * 0.5 * width);
  out[H3DY] = round((H3DVF(1.0) - v[H3DY]) * 0.5 * height);
}

#endif
