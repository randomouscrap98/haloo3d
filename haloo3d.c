// haloopdy 2024

#include "haloo3d.h"
#include "mathc.h"

// ----------------------
//   Vecs and such
// ----------------------

int haloo3d_precalc_verts(haloo3d_obj *obj, mfloat_t *matrix,
                          struct vec4 *out) {
  for (int i = 0; i < obj->numvertices; i++) {
    // eprintf("P: %f %f %f %f\n", obj->vertices[i].x, obj->vertices[i].y,
    // obj->vertices[i].z, obj->vertices[i].w);
    haloo3d_vec4_multmat_into(obj->vertices + i, matrix, out + i);
    // eprintf("P: %f %f %f %f\n", out[i].x, out[i].y, out[i].z, out[i].w);
  }
  return obj->numvertices;
}

// ----------------------
//  Framebuffer
// ----------------------

void haloo3d_fb_init(haloo3d_fb *fb, uint16_t width, uint16_t height) {
  fb->width = width;
  fb->height = height;
  mallocordie(fb->buffer, sizeof(uint16_t) * haloo3d_fb_size(fb));
  mallocordie(fb->wbuffer, sizeof(mfloat_t) * haloo3d_fb_size(fb));
}

void haloo3d_fb_init_tex(haloo3d_fb *fb, uint16_t width, uint16_t height) {
  if (!IS2POW(width) || !IS2POW(height)) {
    dieerr("Texture width and height must be power of 2: %dX%d\n", width,
           height);
  }
  fb->width = width;
  fb->height = height;
  fb->wbuffer = NULL;
  mallocordie(fb->buffer, sizeof(uint16_t) * haloo3d_fb_size(fb));
}

void haloo3d_fb_free(haloo3d_fb *fb) {
  free(fb->buffer);
  if (fb->wbuffer != NULL) {
    free(fb->wbuffer);
  }
}

// ----------------------
//   Camera
// ----------------------

void haloo3d_camera_init(haloo3d_camera *cam) {
  // Initialize the camera to look in a safe direction with
  // reasonable up/etc. You spawn at the origin
  vec3(cam->up.v, 0.0, 1.0, 0.0);
  vec3(cam->pos.v, 0.0, 0.0, 0.0);
  cam->yaw = 0;
  cam->pitch = MPI_2; // This is pi / 2
}

struct vec3 haloo3d_camera_calclook(haloo3d_camera *cam, mfloat_t *view) {
  struct vec3 lookvec;
  struct vec3 lookat;
  // Use sphere equation to compute lookat vector through the two
  // player-controled angles (pitch and yaw)
  vec3(lookvec.v, MSIN(cam->pitch) * MSIN(cam->yaw), MCOS(cam->pitch),
       -MSIN(cam->pitch) * MCOS(cam->yaw));
  vec3_add(lookat.v, cam->pos.v, lookvec.v);
  // eprintf("CAMERAPOS: %f,%f,%f\n", cam->pos.x, cam->pos.y, cam->pos.z);
  // eprintf("CAMERADIR: %f,%f,%f\n", lookvec.x, lookvec.y, lookvec.z);
  // eprintf("LOOKAT: %f,%f,%f\n", lookat.x, lookat.y, lookat.z);
  // eprintf("UP: %f,%f,%f\n", cam->up.x, cam->up.y, cam->up.z);
  // mat4_look_at(view, cam->pos.v, lookat.v, cam->up.v);
  // mat4_look_at(view, cam->pos.v, lookat.v, cam->up.v);
  haloo3d_my_lookat(view, cam->pos.v, lookat.v, cam->up.v);
  return lookvec;
}

void haloo3d_my_lookat(mfloat_t *view, mfloat_t *from, mfloat_t *to,
                       mfloat_t *up) {
  struct vec3 forward;
  struct vec3 right;
  struct vec3 realup;
  vec3_subtract(forward.v, from, to);
  vec3_normalize(forward.v, forward.v);
  vec3_cross(right.v, up, forward.v);
  vec3_normalize(right.v, right.v); // IDK if you have to normalize but whatever
  vec3_cross(realup.v, forward.v, right.v);

  mat4_identity(view);
  view[0] = right.x;
  view[1] = right.y;
  view[2] = right.z;
  view[4] = realup.x;
  view[5] = realup.y;
  view[6] = realup.z;
  view[8] = forward.x;
  view[9] = forward.y;
  view[10] = forward.z;
  view[12] = from[0];
  view[13] = from[1];
  view[14] = from[2];
  // forward := from.Sub(to).Normalize()
  // right := up.CrossProduct(forward).Normalize()
  // realup := forward.CrossProduct(right)
  // m.SetIdentity()
  //  m.Set(0, 0, right.X)
  //  m.Set(1, 0, right.Y)
  //  m.Set(2, 0, right.Z)
  //  m.Set(0, 1, realup.X)
  //  m.Set(1, 1, realup.Y)
  //  m.Set(2, 1, realup.Z)
  //  m.Set(0, 2, forward.X)
  //  m.Set(1, 2, forward.Y)
  //  m.Set(2, 2, forward.Z)
  //  m.Set(0, 3, from.X)
  //  m.Set(1, 3, from.Y)
  //  m.Set(2, 3, from.Z)
}

void haloo3d_perspective(mfloat_t *m, mfloat_t fov, mfloat_t aspect,
                         mfloat_t near, mfloat_t far) {
  mat4_zero(m);
  // func (m *Mat44f) SetProjection(fov float32, aspect float32, near float32,
  // far float32) { m.ZeroFill()

  fov = fov / 180 * MPI; // math.Pi // Convert to radians
  mfloat_t e = 1.0 / MTAN(fov * 0.5);

  m[0] = e / aspect;
  m[5] = e;
  m[10] = (far + near) / (near - far);
  m[11] = -1; // the z divide
  m[14] = 2 * far * near / (near - far);
  // e := float32(1 / math.Tan(float64(fov/2)))

  // m.Set(0, 0, e/aspect)
  // m.Set(1, 1, e)
  // m.Set(2, 2, (far+near)/(near-far))
  // m.Set(2, 3, 2*far*near/(near-far))
  // m.Set(3, 2, -1) // Might need to be swapped
}

// ----------------------
//  Rendering
// ----------------------

void haloo3d_texturedtriangle(haloo3d_fb *fb, haloo3d_fb *texture,
                              mfloat_t intensity, haloo3d_facef face) {
  // min,      max
  struct vec2 boundsTLf =
      haloo3d_boundingbox_tl(face[0].pos.v, face[1].pos.v, face[2].pos.v);
  struct vec2 boundsBRf =
      haloo3d_boundingbox_br(face[0].pos.v, face[1].pos.v, face[2].pos.v);
  // eprintf("Balls: %f,%f %f,%f\n", boundsTLf.x, boundsTLf.y, boundsBRf.x,
  // boundsBRf.y);
  //  The triangle is fully out of bounds; we don't have a proper clipper, so
  //  this check still needs to be performed
  if (boundsBRf.y < 0 || boundsBRf.x < 0 || boundsTLf.x >= fb->width ||
      boundsTLf.y >= fb->height) {
    return;
  }
  struct vec2i v0, v1, v2;
  vec2i_assign_vec2(v0.v, face[0].pos.v);
  vec2i_assign_vec2(v1.v, face[1].pos.v);
  vec2i_assign_vec2(v2.v, face[2].pos.v);
  mint_t parea = haloo3d_edgefunci(v0.v, v1.v, v2.v);
  // Don't even bother with drawing backfaces or degenerate triangles;
  // don't even give the user the option
  if (parea <= 0) {
    return;
  }
  struct vec2i boundsTL = {.x = MAX(boundsTLf.x, 0), .y = MAX(boundsTLf.y, 0)};
  struct vec2i boundsBR = {.x = MIN(boundsBRf.x, fb->width - 1),
                           .y = MIN(boundsBRf.y, fb->height - 1)};
  // boundsTL.x = (boundsTL.x >> 2) << 2;
  // boundsTL.y = (boundsTL.y >> 2) << 2;
  //  BTW our scanning starts at boundsTL
  mfloat_t invarea = 1.0 / parea;
  mint_t w0_y = haloo3d_edgefunci(v1.v, v2.v, boundsTL.v);
  mint_t w1_y = haloo3d_edgefunci(v2.v, v0.v, boundsTL.v);
  mint_t w2_y = haloo3d_edgefunci(v0.v, v1.v, boundsTL.v);
  struct vec2i w0_i = haloo3d_edgeinci(v1.v, v2.v);
  struct vec2i w1_i = haloo3d_edgeinci(v2.v, v0.v);
  struct vec2i w2_i = haloo3d_edgeinci(v0.v, v1.v);
  // I don't know what happened to my z but it's nearly unusable.
  // I simply use w instead... don't know if that's ok
  // TODO: I don't know if z is actually broken; take a look at z fighting
  // and see
  mfloat_t tiz0 = 1.0 / face[0].pos.w;
  mfloat_t tiz1 = 1.0 / face[1].pos.w;
  mfloat_t tiz2 = 1.0 / face[2].pos.w;
  mfloat_t tiu0 = face[0].tex.x * tiz0;
  mfloat_t tiu1 = face[1].tex.x * tiz1;
  mfloat_t tiu2 = face[2].tex.x * tiz2;
  mfloat_t tiv0 = face[0].tex.y * tiz0;
  mfloat_t tiv1 = face[1].tex.y * tiz1;
  mfloat_t tiv2 = face[2].tex.y * tiz2;

  const int yend = boundsBR.y;
  const int xend = boundsBR.x;
  const int ystart = boundsTL.y;
  const int xstart = boundsTL.x;

  for (int y = ystart; y <= yend; y++) {
    mint_t w0 = w0_y;
    mint_t w1 = w1_y;
    mint_t w2 = w2_y;
    for (int x = xstart; x <= xend; x++) {
      if ((w0 | w1 | w2) >= 0) {
        mfloat_t w0a = w0 * invarea;
        mfloat_t w1a = w1 * invarea;
        mfloat_t w2a = w2 * invarea;
        // These are linearly interpolated values and could also be
        // pulled out of the loop, though it may be slower
        mfloat_t pz = w0a * tiz0 + w1a * tiz1 + w2a * tiz2;
        if (pz > haloo3d_wb_get(fb, x, y)) {
          haloo3d_wb_set(fb, x, y, pz);
          pz = 1 / pz;
          uint16_t c = haloo3d_fb_getuv(
              texture, (w0a * tiu0 + w1a * tiu1 + w2a * tiu2) * pz,
              (w0a * tiv0 + w1a * tiv1 + w2a * tiv2) * pz);
          haloo3d_fb_set(fb, x, y, c); // haloo3d_col_scale(c, intensity));
        }
      }
      w0 += w0_i.x;
      w1 += w1_i.x;
      w2 += w2_i.x;
    }
    w0_y += w0_i.y;
    w1_y += w1_i.y;
    w2_y += w2_i.y;
  }
}

int haloo3d_facef_finalize(haloo3d_facef face) {
  // if (face[0].pos.w <= 0 || face[1].pos.w <= 0 || face[2].pos.w <= 0) {
  //   return 0;
  // }
  // void haloo3d_conditional_face(struct vec4 * positions, struct vec3 *
  // vtcoords, haloo3d_facef * out) { sc []HVec3f, tx []Vec3f, out []Facef)
  // []Facef { var f Facef
  // We HAVE to divide points first BEFORE checking the edge function
  haloo3d_vec4_conventional(&face[0].pos); // face[0].pos
  haloo3d_vec4_conventional(&face[1].pos); // face[0].pos
  haloo3d_vec4_conventional(&face[2].pos); // face[0].pos
  return haloo3d_edgefunc(face[0].pos.v, face[1].pos.v, face[2].pos.v) <= 0;

  // for (int i = 0; i < 3; := range 3 {
  // 	f[i].Pos = sc[i].MakeConventional()
  // 	f[i].Tex = tx[i]
  // 	f[i].W = sc[i].W
  // }
  // Backface culling: no need to do anything with triangles facing the wrong
  // way if EdgeFunction(f[0].Pos, f[1].Pos, f[2].Pos) <= 0 { 	out =
  // append(out, f)
  // }

  // return out
}
