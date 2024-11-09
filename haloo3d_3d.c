#include "haloo3d_3d.h"

#include <string.h>

// // ----------------------
// //   Camera
// // ----------------------
//
// void h3d_camera_init(h3d_camera *cam) {
//   // Initialize the camera to look in a safe direction with
//   // reasonable up/etc. You spawn at the origin
//   VEC3(cam->up, 0.0, 1.0, 0.0);
//   VEC3(cam->pos, 0.0, 0.0, 0.0);
//   cam->yaw = 0;
//   cam->pitch = MPI / 2;
// }
//
// void h3d_camera_calclook(h3d_camera *cam, mat4 view, vec3 lookvec) {
//   vec3 lookat;
//   vec3 tmp;
//   if (lookvec == NULL)
//     lookvec = tmp;
//   // Use sphere equation to compute lookat vector through the two
//   // player-controled angles (pitch and yaw)
//   YAWP2VEC(cam->yaw, cam->pitch, lookvec);
//   vec3_add(cam->pos, lookvec, lookat);
//   h3d_my_lookat(cam->pos, lookat, cam->up, view);
// }
//
// void h3d_camera_calcmove_yaw(h3d_camera *cam, vec4 delta) {
//   mat4 rot;
//   mat4_zero(rot);
//   mat4_rotation_y(-cam->yaw, rot);
//   vec4 tmp;
//   memcpy(tmp, delta, sizeof(vec4));
//   h3d_vec4_mult_mat4(tmp, rot, delta);
// }

// ----------------------
//   3d rendering
// ----------------------

void h3d_3dvert_lerp_self(h3d_3dvert *v, h3d_3dvert *v2, int numinterpolants,
                          hfloat_t t) {
  vec4_lerp(v->pos, v2->pos, t, v->pos);
  for (int i = 0; i < numinterpolants; i++) {
    v->interpolants[i] = LERP(v->interpolants[i], v2->interpolants[i], t);
  }
}

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
  view[12] = from[H3DX];
  view[13] = from[H3DY];
  view[14] = from[H3DZ];
  // Remember: I don't pre-invert it. That wastes slightly more time but it
  // lets this function be more useful I think...
}

// My personal perspective matrix setter which uses horizontal fov
// and aspect ratio
void h3d_perspective(hfloat_t fov, hfloat_t aspect, hfloat_t near, hfloat_t far,
                     mat4 m) {
  mat4_zero(m);

  fov = fov / H3DVF(180) * H3DVF(MPI); // Convert to radians
  hfloat_t e = H3DVF(1.0) / tanf(fov * H3DVF(0.5));

  m[0] = e / aspect;
  m[5] = e;
  m[10] = (far + near) / (near - far);
  m[11] = H3DVF(-1); // the z divide
  m[14] = H3DVF(2) * far * near / (near - far);
}

int h3d_3dface_clip(h3d_3dface face, h3d_3dface *out, int numinterpolants) {
  // w + z (back)
  // w - z (front)
  // w + x (left)
  // w - x (right)
  // w + y (bottom)
  // w - y (top)

  // We start with just the one face at index 0
  memcpy(out[0], face, sizeof(h3d_3dface));

#ifdef H3DEBUG_NOCLIPPING
  return 1;
#else
  int outers[3];
  int inners[3];
  vec3 dist;

  // this is our "assignment" tracker. Bits are low to high.
  // We start with the first slot filled
  uint64_t assign = 1;

  // Do for each plane
  for (int p = 0; p < H3D_CLIPPLANES; p++) {
    if (!assign) { // There's no triangles left
      break;
    }
    // How far to move to store triangle 2, also how many tris
    int tris = (1 << p);
    uint64_t tricheck = assign; // local tracker for assign that gets shifted
    for (int t = 0; t < tris; t++) {
      if (!tricheck) { // There's no triangles left
        break;
      }
      if (tricheck & 1) { // Only do tris that are set
        int numinners = 0;
        int numouters = 0;
        h3d_3dvert *f = out[t];
        // Figure out how many are in or out of this plane
        for (int i = 0; i < 3; i++) {
          switch (p) {
          case 0: // z-near
            dist[i] = f[i].pos[H3DW] + f[i].pos[H3DZ];
            break;
          // case 1: // z-far
          //   dist[i] = f[i].pos.w - f[i].pos.z;
          //   break;
          case 1: // x-left
            dist[i] = f[i].pos[H3DW] + f[i].pos[H3DX];
            break;
          case 2: // x-right
            dist[i] = f[i].pos[H3DW] - f[i].pos[H3DX];
            break;
          case 3: // y-bottom
            dist[i] = f[i].pos[H3DW] + f[i].pos[H3DY];
            break;
          case 4: // t-top
            dist[i] = f[i].pos[H3DW] - f[i].pos[H3DY];
            break;
          }
          if (dist[i] < H3D_CLIPLOW) {
            outers[numouters++] = i;
          } else {
            inners[numinners++] = i;
          }
        }
        // Now we know how many points are inside or out against this one plane
        if (numouters == 3) { // This is rejected fully, clear the assign
          assign &= ~(1L << t);
        } else if (numouters == 2) { // The one triangle thing
          // two points are outside; the point as stored is fine, but we need to
          // fix a couple things.
          int ai = inners[0];
          int bi = outers[0];
          int ci = outers[1];
          // Calc how far along we are on each of these lines. These are the new
          // points
          // NOTE: we nudge it a little forward to prevent weird issues
          hfloat_t tba = dist[bi] / (dist[bi] - dist[ai]) + H3D_CLIPLOW;
          hfloat_t tca = dist[ci] / (dist[ci] - dist[ai]) + H3D_CLIPLOW;
          // The two points that aren't 'a' need to be the interpolated values
          h3d_3dvert_lerp_self(f + bi, f + ai, numinterpolants, tba);
          h3d_3dvert_lerp_self(f + ci, f + ai, numinterpolants, tca);
          // Don't do anything with assign, it's already set for this tri
        } else if (numouters == 1) { // The two triangle thing
          // For this one, we need to mutate the original AND produce a new
          int ai = outers[0]; // A is the odd one out
          int bi = inners[0];
          int ci = inners[1];
          hfloat_t tab = dist[ai] / (dist[ai] - dist[bi]) + H3D_CLIPLOW;
          hfloat_t tac = dist[ai] / (dist[ai] - dist[ci]) + H3D_CLIPLOW;
          h3d_3dvert *f2 = out[t + tris];
          // BEFORE modification, we copy the existing triangle to the final
          // outer place
          memcpy(f2, f, sizeof(h3d_3dface));
          assign |= (1L << (t + tris));
          // Fix existing triangle by replacing the bad outer point a
          // with an interpolated one to b
          h3d_3dvert_lerp_self(f + ai, f + bi, numinterpolants, tab);
          f2[bi] = f[ai]; // Swap the old a point to b
          // And once again replace the a point but interpolating with c
          h3d_3dvert_lerp_self(f2 + ai, f2 + ci, numinterpolants, tac);
        }
        // Don't need to check for trivial accept, it's already where it needs
        // to be
      }
      // this is the end of the triangle loop, move to the next triangle
      tricheck >>= 1;
    }
  }

  // Now that we're here, we need to backfill the holes
  int numout = 0;
  int index = 0;

  // Though this could be expensive, generally speaking, there won't be
  // many triangles here. It doesn't scale well though. This condenses the
  // triangles assigned to specific slots back into sequential order
  while (assign) {
    if (assign & 1) {
      if (index != numout) {
        memcpy(out + numout, out + index, sizeof(h3d_3dface));
      }
      numout++;
    }
    assign >>= 1;
    index++;
  }

  return numout;
#endif
}

void h3d_model_matrix(vec3 pos, vec3 lookvec, vec3 up, vec3 scale, mat4 out) {
  vec4 tmp;
  vec3_add(pos, lookvec, tmp);
  h3d_my_lookat(pos, tmp, up, out);
  // Apply scale such that it looks like it was applied first (this prevents
  // scaling applying skew to a rotated object)
  h3d_mat4_prescale_self(out, scale);
}
