// haloopdy 2024

#include "haloo3d.h"
#include "mathc.h"
#include <string.h>

// ----------------------
//   Vecs and such
// ----------------------

int haloo3d_precalc_verts(haloo3d_obj *obj, mfloat_t *matrix,
                          struct vec4 *out) {
  for (int i = 0; i < obj->numvertices; i++) {
    haloo3d_vec4_multmat_into(obj->vertices + i, matrix, out + i);
  }
  return obj->numvertices;
}

// TODO: optimize whole light calc (including normals) to use mostly integers
mfloat_t haloo3d_calc_light(mfloat_t *light, mfloat_t minlight,
                            haloo3d_facef face) {
  mfloat_t lnorm[VEC3_SIZE];
  haloo3d_facef_normal(face, lnorm);
  mfloat_t intensity =
      light[0] * lnorm[0] + light[1] * lnorm[1] + light[2] * lnorm[2];
  if (intensity < minlight) {
    return minlight; // Don't just not draw the triangle: it should be black
  } else {
    return (intensity + minlight) / (1 + minlight);
  }
}

void haloo3d_objin_init(haloo3d_obj_instance *obj, haloo3d_obj *model,
                        haloo3d_fb *tex) {
  obj->model = model;
  obj->texture = tex;
  obj->scale = 1.0;
  obj->color = 0xFFFF;
  obj->lighting = NULL; // Default no lighting
  vec3(obj->pos.v, 0, 0, 0);
  // Assume user is going to use lookvec as a facing offset and not a raw
  // lookat. Though, this value will work regardless, considering the position
  vec3(obj->lookvec.v, 0, 0, -1);
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
  // Remember: I don't pre-invert it. That wastes slightly more time but it
  // lets this function be more useful I think...
}

// My personal perspective matrix setter which uses horizontal fov
// and aspect ratio
void haloo3d_perspective(mfloat_t *m, mfloat_t fov, mfloat_t aspect,
                         mfloat_t near, mfloat_t far) {
  mat4_zero(m);

  fov = fov / 180 * MPI; // math.Pi // Convert to radians
  mfloat_t e = 1.0 / MTAN(fov * 0.5);

  m[0] = e / aspect;
  m[5] = e;
  m[10] = (far + near) / (near - far);
  m[11] = -1; // the z divide
  // m[14] = (far * near - near) / (near - far);
  m[14] = 2 * far * near / (near - far);
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
  //  BTW our scanning starts at boundsTL
  // int32_t invarea = (1.0 / parea) * (1 << _H3D_RS);
  mfloat_t invarea = 1.0 / parea;
  mint_t w0_y = haloo3d_edgefunci(v1.v, v2.v, boundsTL.v);
  mint_t w1_y = haloo3d_edgefunci(v2.v, v0.v, boundsTL.v);
  mint_t w2_y = haloo3d_edgefunci(v0.v, v1.v, boundsTL.v);
  struct vec2i w0_i = haloo3d_edgeinci(v1.v, v2.v);
  struct vec2i w1_i = haloo3d_edgeinci(v2.v, v0.v);
  struct vec2i w2_i = haloo3d_edgeinci(v0.v, v1.v);
  // Cant use z because it's -1 to 1. w is nicer to work with since it's near to
  // far and can never be 0.
  mfloat_t tiz0 = 1.0 / face[0].pos.w;
  mfloat_t tiz1 = 1.0 / face[1].pos.w;
  mfloat_t tiz2 = 1.0 / face[2].pos.w;
  mfloat_t tiu0 = face[0].tex.x * tiz0;
  mfloat_t tiu1 = face[1].tex.x * tiz1;
  mfloat_t tiu2 = face[2].tex.x * tiz2;
  mfloat_t tiv0 = face[0].tex.y * tiz0;
  mfloat_t tiv1 = face[1].tex.y * tiz1;
  mfloat_t tiv2 = face[2].tex.y * tiz2;

#ifdef H3D_DEBUGCLIP
  eprintf("Z: %f %f %f\n", face[0].pos.z, face[1].pos.z, face[2].pos.z);
  eprintf("W: %f %f %f\n", face[0].pos.w, face[1].pos.w, face[2].pos.w);
  eprintf("UV0: %f,%f UV1: %f,%f UV2: %f,%f\n", face[0].tex.x, face[0].tex.y,
          face[1].tex.x, face[1].tex.y, face[2].tex.x, face[2].tex.y);
  // eprintf("TIZ: %f %f %f\n", tiz0, tiz1, tiz2);
#endif
  //  int32_t tiz0 = (1.0 / face[0].pos.w) * (1 << _H3D_RS);
  //  int32_t tiz1 = (1.0 / face[1].pos.w) * (1 << _H3D_RS);
  //  int32_t tiz2 = (1.0 / face[2].pos.w) * (1 << _H3D_RS);
  //  int32_t tiu0 = (face[0].tex.x * tiz0) * (1 << _H3D_RS);
  //  int32_t tiu1 = (face[1].tex.x * tiz1) * (1 << _H3D_RS);
  //  int32_t tiu2 = (face[2].tex.x * tiz2) * (1 << _H3D_RS);
  //  int32_t tiv0 = (face[0].tex.y * tiz0) * (1 << _H3D_RS);
  //  int32_t tiv1 = (face[1].tex.y * tiz1) * (1 << _H3D_RS);
  //  int32_t tiv2 = (face[2].tex.y * tiz2) * (1 << _H3D_RS);

  const int yend = boundsBR.y;
  const int xend = boundsBR.x;
  const int ystart = boundsTL.y;
  const int xstart = boundsTL.x;
  const uint16_t scale = intensity * 256;

  for (int y = ystart; y <= yend; y++) {
    mint_t w0 = w0_y;
    mint_t w1 = w1_y;
    mint_t w2 = w2_y;
    for (int x = xstart; x <= xend; x++) {
      if ((w0 | w1 | w2) >= 0) {
        mfloat_t pz = (w0 * tiz0 + w1 * tiz1 + w2 * tiz2) * invarea;
        if (pz > haloo3d_wb_get(fb, x, y)) {
          mfloat_t pcz = invarea / pz;
          uint16_t c = haloo3d_fb_getuv(
              texture, (w0 * tiu0 + w1 * tiu1 + w2 * tiu2) * pcz,
              (w0 * tiv0 + w1 * tiv1 + w2 * tiv2) * pcz);
          if (c & 0xF000) {
            haloo3d_wb_set(fb, x, y, pz);
            haloo3d_fb_set(fb, x, y, haloo3d_col_scalei(c, scale));
          }
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
  // We HAVE to divide points first BEFORE checking the edge function
  haloo3d_vec4_conventional(&face[0].pos); // face[0].pos
  haloo3d_vec4_conventional(&face[1].pos); // face[0].pos
  haloo3d_vec4_conventional(&face[2].pos); // face[0].pos
  return haloo3d_edgefunc(face[0].pos.v, face[1].pos.v, face[2].pos.v) <= 0;
}

// NOTE: this function only clips against the near plane!
int haloo3d_facef_clip(haloo3d_facef face, haloo3d_facef *out) {
  int numout = 0;
  int outers[3];
  int inners[3];
  int numinners = 0;
  int numouters = 0;
  mfloat_t dist[3];

  for (int i = 0; i < 3; i++) {
    dist[i] = face[i].pos.z + face[i].pos.w;
    if (dist[i] < H3D_FACEF_CLIPLOW) { //< H3D_FACEF_CLIPLOW) {
      outers[numouters++] = i;
    } else {
      inners[numinners++] = i;
    }
  }

#ifdef H3D_DEBUGCLIP
  eprintf("---------------------------------\n");
  eprintf("0p0: %f,%f,%f(%f) 0p1: %f,%f,%f(%f) 0p2: %f,%f,%f(%f)\n",
          face[0].pos.x, face[0].pos.y, face[0].pos.z, face[0].pos.w,
          face[1].pos.x, face[0].pos.y, face[1].pos.z, face[1].pos.w,
          face[2].pos.x, face[2].pos.y, face[2].pos.z, face[2].pos.w);
#endif

  if (numouters == 2) { // The one triangle thing
    // We CAN use the face (sort of), so copy it out
    memcpy(out[numout], face, sizeof(haloo3d_facef));

    int ai = inners[0];
    int bi = outers[0];
    int ci = outers[1];

    // Calc how far along we are on each of these lines. These are the new
    // points
    // NOTE: we nudge it a little forward to prevent weird issues
    mfloat_t tba = dist[bi] / (dist[bi] - dist[ai]) + H3D_FACEF_CLIPLOW;
    mfloat_t tca = dist[ci] / (dist[ci] - dist[ai]) + H3D_FACEF_CLIPLOW;

    // The two points that aren't 'a' need to be the interpolated values
    haloo3d_vertexf_lerp_self(&out[numout][bi], &face[ai], tba);
    haloo3d_vertexf_lerp_self(&out[numout][ci], &face[ai], tca);

#ifdef H3D_DEBUGCLIP
    eprintf("pA: %f,%f,%f pB: %f,%f,%f pC: %f,%f,%f\n", out[numout][ai].pos.x,
            out[numout][ai].pos.y, out[numout][ai].pos.z, out[numout][bi].pos.x,
            out[numout][bi].pos.y, out[numout][bi].pos.z, out[numout][ci].pos.x,
            out[numout][ci].pos.y, out[numout][ci].pos.z);
    eprintf("DA: %f DB: %f DC: %f\n", dist[ai], dist[bi], dist[ci]);
    eprintf("TBA: %f TCA: %f\n", tba, tca);
    eprintf("UVA: %f,%f UVB: %f,%f UVC: %f,%f\n", out[numout][ai].tex.x,
            out[numout][ai].tex.y, out[numout][bi].tex.x, out[numout][bi].tex.y,
            out[numout][ci].tex.x, out[numout][ci].tex.y);
#endif

    if (haloo3d_facef_finalize(out[numout])) {
      numout++;
    }
  } else if (numouters == 1) { // The two triangle thing

    int ai = outers[0]; // A is the odd one out
    int bi = inners[0];
    int ci = inners[1];

    mfloat_t tab = dist[ai] / (dist[ai] - dist[bi]) + H3D_FACEF_CLIPLOW;
    mfloat_t tac = dist[ai] / (dist[ai] - dist[ci]) + H3D_FACEF_CLIPLOW;

#ifdef H3D_DEBUGCLIP
    eprintf("TAB: %f, TAC: %f\n", tab, tac);
#endif

    // Generate the first new triangle by replacing the bad outer point a
    // with an interpolated one to b
    memcpy(out[numout], face, sizeof(haloo3d_facef));
    haloo3d_vertexf_lerp_self(out[numout] + ai, out[numout] + bi, tab);

    // haloo3d_vertexf olda;
    // memcpy(&olda, out[numout] + ai, sizeof(haloo3d_vertexf));
    haloo3d_vertexf olda = out[numout][ai];

    if (haloo3d_facef_finalize(out[numout])) {
      numout++;
    }

    // We RESET the next point to simplify the process, and once again replace
    // the a point but interpolating with c
    memcpy(out[numout], face, sizeof(haloo3d_facef));
    haloo3d_vertexf_lerp_self(out[numout] + ai, out[numout] + ci, tac);

    // But the B point needs to actually be the interpolated A point
    // memcpy(out[numout] + bi, &olda, sizeof(haloo3d_vertexf));
    out[numout][bi] = olda;

    if (haloo3d_facef_finalize(out[numout])) {
      numout++;
    }

  } else if (numouters == 0) { // Output the face itself, no modification
    memcpy(out[numout], face, sizeof(haloo3d_facef));
#ifdef H3D_DEBUGCLIP
    eprintf("p0: %f,%f,%f p1: %f,%f,%f p2: %f,%f,%f\n", out[numout][0].pos.x,
            out[numout][0].pos.y, out[numout][0].pos.z, out[numout][1].pos.x,
            out[numout][0].pos.y, out[numout][1].pos.z, out[numout][2].pos.x,
            out[numout][2].pos.y, out[numout][2].pos.z);
#endif
    if (haloo3d_facef_finalize(out[numout])) {
      numout++;
    }
  }

  return numout;
}

void haloo3d_sprite(haloo3d_fb *fb, haloo3d_fb *sprite, haloo3d_recti texrect,
                    haloo3d_recti outrect) {
  // Precalc the step, as it's always the same even if we clip the rect
  const int FIXEDPOINTDEPTH = H3D_SPRITE_FPDEPTH;
  struct vec2i texdim = haloo3d_recti_dims(&texrect);
  struct vec2i outdim = haloo3d_recti_dims(&outrect);
  int32_t stepx = (1 << FIXEDPOINTDEPTH) * (float)texdim.x / outdim.x;
  int32_t stepy = (1 << FIXEDPOINTDEPTH) * (float)texdim.y / outdim.y;
  int32_t texx = (1 << FIXEDPOINTDEPTH) * texrect.x1;
  int32_t texy = (1 << FIXEDPOINTDEPTH) * texrect.y1;
  // Clip the rect
  if (outrect.x1 < 0) {
    texx += stepx * -outrect.x1;
    outrect.x1 = 0;
  }
  if (outrect.y1 < 0) {
    texy += stepy * -outrect.y1;
    outrect.y1 = 0;
  }
  if (outrect.x2 >= fb->width) {
    outrect.x2 = fb->width - 1;
  }
  if (outrect.y2 >= fb->height) {
    outrect.y2 = fb->height - 1;
  }

  for (int y = outrect.y1; y < outrect.y2; y++) {
    texx = texrect.x1;
    for (int x = outrect.x1; x < outrect.x2; x++) {
      uint16_t pix = haloo3d_fb_get(sprite, texx >> FIXEDPOINTDEPTH,
                                    texy >> FIXEDPOINTDEPTH);
      if (pix >> 12) {
        haloo3d_fb_set(fb, x, y, pix);
      }
      texx += stepx;
    }
    texy += stepy;
  }
}
