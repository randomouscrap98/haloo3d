#ifndef __HALOO3D_MATH_H
#define __HALOO3D_MATH_H

#include "haloo3d.h"
#include <math.h>

#define MPI 3.1415926536f

// The maximum amount of faces produced by clipping,
// though realistically this is never reached (also I don't
// even know if that's correct). It should(?) be that for
// each plane we clip against, we multiply by two. So
// however many planes we clip, that's the number we shift
#define H3D_FACEF_CLIPPLANES 5
#define H3D_FACEF_MAXCLIP (1 << H3D_FACEF_CLIPPLANES)
#define H3D_FACEF_CLIPLOW H3DVF(0.0)

#define LERP(a, b, t) ((a) + ((b) - (a)) * (t))
// NOTE: pitch = 0 means pointing straight up, this is to prevent gimbal
// lock!
#define YAWP2VEC(yaw, pitch, out)                                              \
  VEC3(out, sinf(pitch) * sinf(yaw), cosf(pitch), -sinf(pitch) * cosf(yaw));

// typedef vec4 h3d_face[3];

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

// Linear interpolate all values between v0 and v1 based on t. Result
// can safely be either v1 or v0
static inline void vec3_lerp(vec3 v0, vec3 v1, float_t t, vec3 result) {
  result[0] = LERP(v0[0], v1[0], t);
  result[1] = LERP(v0[1], v1[1], t);
  result[2] = LERP(v0[2], v1[2], t);
}

// Linear interpolate all values between v0 and v1 based on t. Result
// can safely be either v1 or v0
static inline void vec4_lerp(vec4 v0, vec4 v1, float_t t, vec4 result) {
  result[0] = LERP(v0[0], v1[0], t);
  result[1] = LERP(v0[1], v1[1], t);
  result[2] = LERP(v0[2], v1[2], t);
  result[3] = LERP(v0[3], v1[3], t);
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

// Multiply m0 by m1, storing the result in result. Result can be either of the
// inputs safely. The row/column order technically doesn't matter, because it
// matters how you put the numbers into memory. For what it's worth, it does
// columns on m0 multiplied by rows on m1
static inline void mat4_multiply(mat4 m0, mat4 m1, mat4 result) {
  mat4 multiplied;
  multiplied[0] =
      m0[0] * m1[0] + m0[4] * m1[1] + m0[8] * m1[2] + m0[12] * m1[3];
  multiplied[1] =
      m0[1] * m1[0] + m0[5] * m1[1] + m0[9] * m1[2] + m0[13] * m1[3];
  multiplied[2] =
      m0[2] * m1[0] + m0[6] * m1[1] + m0[10] * m1[2] + m0[14] * m1[3];
  multiplied[3] =
      m0[3] * m1[0] + m0[7] * m1[1] + m0[11] * m1[2] + m0[15] * m1[3];
  multiplied[4] =
      m0[0] * m1[4] + m0[4] * m1[5] + m0[8] * m1[6] + m0[12] * m1[7];
  multiplied[5] =
      m0[1] * m1[4] + m0[5] * m1[5] + m0[9] * m1[6] + m0[13] * m1[7];
  multiplied[6] =
      m0[2] * m1[4] + m0[6] * m1[5] + m0[10] * m1[6] + m0[14] * m1[7];
  multiplied[7] =
      m0[3] * m1[4] + m0[7] * m1[5] + m0[11] * m1[6] + m0[15] * m1[7];
  multiplied[8] =
      m0[0] * m1[8] + m0[4] * m1[9] + m0[8] * m1[10] + m0[12] * m1[11];
  multiplied[9] =
      m0[1] * m1[8] + m0[5] * m1[9] + m0[9] * m1[10] + m0[13] * m1[11];
  multiplied[10] =
      m0[2] * m1[8] + m0[6] * m1[9] + m0[10] * m1[10] + m0[14] * m1[11];
  multiplied[11] =
      m0[3] * m1[8] + m0[7] * m1[9] + m0[11] * m1[10] + m0[15] * m1[11];
  multiplied[12] =
      m0[0] * m1[12] + m0[4] * m1[13] + m0[8] * m1[14] + m0[12] * m1[15];
  multiplied[13] =
      m0[1] * m1[12] + m0[5] * m1[13] + m0[9] * m1[14] + m0[13] * m1[15];
  multiplied[14] =
      m0[2] * m1[12] + m0[6] * m1[13] + m0[10] * m1[14] + m0[14] * m1[15];
  multiplied[15] =
      m0[3] * m1[12] + m0[7] * m1[13] + m0[11] * m1[14] + m0[15] * m1[15];
  result[0] = multiplied[0];
  result[1] = multiplied[1];
  result[2] = multiplied[2];
  result[3] = multiplied[3];
  result[4] = multiplied[4];
  result[5] = multiplied[5];
  result[6] = multiplied[6];
  result[7] = multiplied[7];
  result[8] = multiplied[8];
  result[9] = multiplied[9];
  result[10] = multiplied[10];
  result[11] = multiplied[11];
  result[12] = multiplied[12];
  result[13] = multiplied[13];
  result[14] = multiplied[14];
  result[15] = multiplied[15];
}

// Calculate the inverse of m0 and put result in result. They can be
// the same variable
static inline void mat4_inverse(mat4 m0, mat4 result) {
  mat4 inverse;
  float_t inverted_determinant;
  float_t m11 = m0[0];
  float_t m21 = m0[1];
  float_t m31 = m0[2];
  float_t m41 = m0[3];
  float_t m12 = m0[4];
  float_t m22 = m0[5];
  float_t m32 = m0[6];
  float_t m42 = m0[7];
  float_t m13 = m0[8];
  float_t m23 = m0[9];
  float_t m33 = m0[10];
  float_t m43 = m0[11];
  float_t m14 = m0[12];
  float_t m24 = m0[13];
  float_t m34 = m0[14];
  float_t m44 = m0[15];
  inverse[0] = m22 * m33 * m44 - m22 * m43 * m34 - m23 * m32 * m44 +
               m23 * m42 * m34 + m24 * m32 * m43 - m24 * m42 * m33;
  inverse[4] = -m12 * m33 * m44 + m12 * m43 * m34 + m13 * m32 * m44 -
               m13 * m42 * m34 - m14 * m32 * m43 + m14 * m42 * m33;
  inverse[8] = m12 * m23 * m44 - m12 * m43 * m24 - m13 * m22 * m44 +
               m13 * m42 * m24 + m14 * m22 * m43 - m14 * m42 * m23;
  inverse[12] = -m12 * m23 * m34 + m12 * m33 * m24 + m13 * m22 * m34 -
                m13 * m32 * m24 - m14 * m22 * m33 + m14 * m32 * m23;
  inverse[1] = -m21 * m33 * m44 + m21 * m43 * m34 + m23 * m31 * m44 -
               m23 * m41 * m34 - m24 * m31 * m43 + m24 * m41 * m33;
  inverse[5] = m11 * m33 * m44 - m11 * m43 * m34 - m13 * m31 * m44 +
               m13 * m41 * m34 + m14 * m31 * m43 - m14 * m41 * m33;
  inverse[9] = -m11 * m23 * m44 + m11 * m43 * m24 + m13 * m21 * m44 -
               m13 * m41 * m24 - m14 * m21 * m43 + m14 * m41 * m23;
  inverse[13] = m11 * m23 * m34 - m11 * m33 * m24 - m13 * m21 * m34 +
                m13 * m31 * m24 + m14 * m21 * m33 - m14 * m31 * m23;
  inverse[2] = m21 * m32 * m44 - m21 * m42 * m34 - m22 * m31 * m44 +
               m22 * m41 * m34 + m24 * m31 * m42 - m24 * m41 * m32;
  inverse[6] = -m11 * m32 * m44 + m11 * m42 * m34 + m12 * m31 * m44 -
               m12 * m41 * m34 - m14 * m31 * m42 + m14 * m41 * m32;
  inverse[10] = m11 * m22 * m44 - m11 * m42 * m24 - m12 * m21 * m44 +
                m12 * m41 * m24 + m14 * m21 * m42 - m14 * m41 * m22;
  inverse[14] = -m11 * m22 * m34 + m11 * m32 * m24 + m12 * m21 * m34 -
                m12 * m31 * m24 - m14 * m21 * m32 + m14 * m31 * m22;
  inverse[3] = -m21 * m32 * m43 + m21 * m42 * m33 + m22 * m31 * m43 -
               m22 * m41 * m33 - m23 * m31 * m42 + m23 * m41 * m32;
  inverse[7] = m11 * m32 * m43 - m11 * m42 * m33 - m12 * m31 * m43 +
               m12 * m41 * m33 + m13 * m31 * m42 - m13 * m41 * m32;
  inverse[11] = -m11 * m22 * m43 + m11 * m42 * m23 + m12 * m21 * m43 -
                m12 * m41 * m23 - m13 * m21 * m42 + m13 * m41 * m22;
  inverse[15] = m11 * m22 * m33 - m11 * m32 * m23 - m12 * m21 * m33 +
                m12 * m31 * m23 + m13 * m21 * m32 - m13 * m31 * m22;
  inverted_determinant = H3DVF(1.0) / (m11 * inverse[0] + m21 * inverse[4] +
                                       m31 * inverse[8] + m41 * inverse[12]);
  result[0] = inverse[0] * inverted_determinant;
  result[1] = inverse[1] * inverted_determinant;
  result[2] = inverse[2] * inverted_determinant;
  result[3] = inverse[3] * inverted_determinant;
  result[4] = inverse[4] * inverted_determinant;
  result[5] = inverse[5] * inverted_determinant;
  result[6] = inverse[6] * inverted_determinant;
  result[7] = inverse[7] * inverted_determinant;
  result[8] = inverse[8] * inverted_determinant;
  result[9] = inverse[9] * inverted_determinant;
  result[10] = inverse[10] * inverted_determinant;
  result[11] = inverse[11] * inverted_determinant;
  result[12] = inverse[12] * inverted_determinant;
  result[13] = inverse[13] * inverted_determinant;
  result[14] = inverse[14] * inverted_determinant;
  result[15] = inverse[15] * inverted_determinant;
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

// // ----------------------
// //   Camera
// // ----------------------
//
// // Describes a camera which can rotate in two directions only (no rolling)
// typedef struct {
//   vec3 pos;
//   vec3 up;
//   float_t pitch;
//   float_t yaw;
// } h3d_camera;
//
// // Initialize the camera to look in a safe direction with
// // reasonable up/etc. You spawn at the origin
// void h3d_camera_init(h3d_camera *cam);
//
// // Calculate the look vector (last param), using it to set the given matrix
// view
// // to the "look_at" matrix
// void h3d_camera_calclook(h3d_camera *cam, mat4 view, vec3 lookvec);
//
// // Transform given vector by yaw rotation indicated in camera.
// // TODO: this function might be unnecessary bloat
// void h3d_camera_calcmove_yaw(h3d_camera *cam, vec4 delta);

// ----------------------
//   3d rendering
// ----------------------

// All the data associated with a single 3d vertex. It will/can be used in
// various processing.
typedef struct {
  vec4 pos;
  float_t interpolants[H3D_MAXINTERPOLANTS];
} h3d_3dvert;

typedef h3d_3dvert h3d_3dface[3];

// Linear interpolate between v and v2, storing result back into v. Also
// lerps the interpolants.
void h3d_3dvert_lerp_self(h3d_3dvert *v, h3d_3dvert *v2, int numinterpolants,
                          float_t t);

// My personal lookat function, which does not perform the inverse or anything.
// Because of this, you can use it to orient models too
void h3d_my_lookat(vec3 from, vec3 to, vec3 up, mat4 view);

// My personal perspective projection function. For some reason, it produces
// different results than the mathc libary's
void h3d_perspective(float_t fov, float_t aspect, float_t near, float_t far,
                     mat4 m);

// Calculate triangle viewport pixel coordinates from normalized coordinates
static inline void h3d_viewport(float_t *v, int width, int height,
                                int16_t *out) {
  // I'm honestly not sure if round, ceil, or floor is more appropriate.
  // I've read those articles from Chris Hecker indicating that ceil
  // should be used, but it didn't seem to produce the right results.
  out[H3DX] = round((v[H3DX] + H3DVF(1.0)) * 0.5 * width);
  out[H3DY] = round((H3DVF(1.0) - v[H3DY]) * 0.5 * height);
}

// Take a translated, potentially clipped single face and fix all the points
// to be normalized to w = 1 again. Returns whether the triangle is facing
// the camera (?)
static inline int h3d_3dface_normalize(h3d_3dface face) {
  // We HAVE to divide points first BEFORE checking the edge function
  h3d_vec4_homogenous(face[0].pos);
  h3d_vec4_homogenous(face[1].pos);
  h3d_vec4_homogenous(face[2].pos);
  return H3D_EDGEFUNC(face[0].pos, face[1].pos, face[2].pos) <= 0;
}

// Clip a single face into 0-2^CLIPPLANES (currently 32) new faces. Make sure
// the out array has enough space (H3D_FACEF_MAXCLIP)
int h3d_3dface_clip(h3d_3dface face, h3d_3dface *out, int numinterpolants);

// Create model matrix from simple position, lookvector (vector describing
// facing direction in world), up vector, and scale
void h3d_model_matrix(vec3 pos, vec3 lookvec, vec3 up, vec3 scale, mat4 out);

#endif
