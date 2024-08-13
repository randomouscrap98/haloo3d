// haloopdy 2024

#include "haloo3d.h"
#include "lib/mathc.h"
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
  obj->cullbackface = 1;
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
  mallocordie(fb->dbuffer, sizeof(mfloat_t) * haloo3d_fb_size(fb));
}

void haloo3d_fb_init_tex(haloo3d_fb *fb, uint16_t width, uint16_t height) {
  if (!IS2POW(width) || !IS2POW(height)) {
    dieerr("Texture width and height must be power of 2: %dX%d\n", width,
           height);
  }
  fb->width = width;
  fb->height = height;
  fb->dbuffer = NULL;
  mallocordie(fb->buffer, sizeof(uint16_t) * haloo3d_fb_size(fb));
}

void haloo3d_fb_free(haloo3d_fb *fb) {
  free(fb->buffer);
  if (fb->dbuffer != NULL) {
    free(fb->dbuffer);
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

void haloo3d_camera_calcmove_yaw(haloo3d_camera *cam, struct vec4 *delta) {
  mfloat_t rot[MAT4_SIZE];
  mat4_zero(rot);
  mat4_rotation_x(rot, -cam->yaw);
  struct vec4 result = haloo3d_vec4_multmat(delta, rot);
  *delta = result;
}

// ----------------------
//  Rendering
// ----------------------

void haloo3d_texturedtriangle(haloo3d_fb *fb, haloo3d_fb *texture,
                              mfloat_t intensity, haloo3d_facef face) {
  haloo3d_vertexf v0v = face[0];
  haloo3d_vertexf v1v = face[1];
  haloo3d_vertexf v2v = face[2];
  // min,      max
  struct vec2 boundsTLf =
      haloo3d_boundingbox_tl(v0v.pos.v, v1v.pos.v, v2v.pos.v);
  struct vec2 boundsBRf =
      haloo3d_boundingbox_br(v0v.pos.v, v1v.pos.v, v2v.pos.v);
  struct vec2i v0, v1, v2;
  vec2i_assign_vec2(v0.v, v0v.pos.v);
  vec2i_assign_vec2(v1.v, v1v.pos.v);
  vec2i_assign_vec2(v2.v, v2v.pos.v);
  mint_t parea = haloo3d_edgefunci(v0.v, v1.v, v2.v);
  // The user decides if they want backface culling before they call this func
  if (parea == 0) {
    return;
  } else if (parea < 0) {
    // swap any two vertices and continue
    struct vec2i tmp = v0;
    v0 = v1;
    v1 = tmp;
    haloo3d_vertexf tmpv = v0v;
    v0v = v1v;
    v1v = tmpv;
    parea = -parea;
  }
  // struct vec2i boundsTL = {.x = MAX(boundsTLf.x, 0), .y = MAX(boundsTLf.y,
  // 0)}; struct vec2i boundsBR = {.x = MIN(boundsBRf.x, fb->width - 1),
  //                          .y = MIN(boundsBRf.y, fb->height - 1)};
  struct vec2i boundsTL = {.x = boundsTLf.x, .y = boundsTLf.y};
  struct vec2i boundsBR = {.x = boundsBRf.x, .y = boundsBRf.y};
  if (boundsTL.x < 0 || boundsBR.x < 0 || boundsTL.y < 0 || boundsBR.y < 0 ||
      boundsTL.x >= fb->width || boundsBR.x >= fb->width ||
      boundsTL.y >= fb->height || boundsBR.y >= fb->height) {
    dieerr("YOU SUCK: (%d,%d)->(%d,%d)", boundsTL.x, boundsTL.y, boundsBR.x,
           boundsBR.y);
  }
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
  mfloat_t tiz0 = 1.0 / v0v.pos.w;
  mfloat_t tiz1 = 1.0 / v1v.pos.w;
  mfloat_t tiz2 = 1.0 / v2v.pos.w;
  mfloat_t tiu0 = v0v.tex.x * tiz0;
  mfloat_t tiu1 = v1v.tex.x * tiz1;
  mfloat_t tiu2 = v2v.tex.x * tiz2;
  mfloat_t tiv0 = v0v.tex.y * tiz0;
  mfloat_t tiv1 = v1v.tex.y * tiz1;
  mfloat_t tiv2 = v2v.tex.y * tiz2;

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
        // This value HAS to be normalized to be useful in the buffer!!!
        mfloat_t pz = (w0 * tiz0 + w1 * tiz1 + w2 * tiz2) * invarea;
        if (pz > haloo3d_db_get(fb, x, y)) {
          mfloat_t pcz = invarea / pz;
          uint16_t c = haloo3d_fb_getuv(
              texture, (w0 * tiu0 + w1 * tiu1 + w2 * tiu2) * pcz,
              (w0 * tiv0 + w1 * tiv1 + w2 * tiv2) * pcz);
          if (c & 0xF000) {
            haloo3d_db_set(fb, x, y, pz);
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

// Represents tracking information for one side of a triangle.
// It may not use or calculate all fields; the left sides require
// the interpolation and the right side only needs x. It uses a
// stack of vectors setup by determining the "direction" of the
// scanline; the "stack" is popped when a vector section is complete
typedef struct {
  haloo3d_vertexf *stack[3];
  uint16_t twidth;
  uint16_t theight;
  int top;
  int sectionheight; // Tracking for how much is left in the current section
  int trackall;
  // These are 16.16 fixed point kinda
  int32_t x, u, v, z;     // Tracking variables
  int32_t dx, du, dv, dz; // Delta along CURRENT edge
  // mfloat_t z;
  // mfloat_t dz;
} _h3dtriside;

static inline void _h3dtriside_init(_h3dtriside *s, haloo3d_fb *texture) {
  s->top = 0;
  s->trackall = 0;
  s->sectionheight = 0;
  s->twidth = texture->width;
  s->theight = texture->height;
}

// Push a normal vector onto the vector stack. Remember it is a stack,
// so the ones you want on top should go last
static inline int _h3dtriside_push(_h3dtriside *s, haloo3d_vertexf *v) {
  s->stack[s->top] = v;
  return ++s->top;
}

// Pop a vector off the stack, returning the new top. Top is technically
// the size of the stack; if 0, it is empty
static inline int _h3dtriside_pop(_h3dtriside *s) { return --s->top; }

// Calculate all deltas (or at least all set to track) and return
// the height of this section.
static inline int _h3dtriside_start(_h3dtriside *s) {
  const haloo3d_vertexf *const v1 = s->stack[s->top - 1];
  const haloo3d_vertexf *const v2 = s->stack[s->top - 2];
  const int height =
      v2->pos.y - v1->pos.y; // this might throw away info, that's ok
  if (height == 0) {
    return 0;
  }
  s->dx = H3D_FP16(v2->pos.x - v1->pos.x) / height;
  s->x = H3D_FP16(v1->pos.x);
  if (s->trackall) {
    s->du = H3D_FP16((v2->tex.x - v1->tex.x) * s->twidth) / height;
    s->u = H3D_FP16(v1->tex.x * s->twidth);
    s->dv = H3D_FP16((v2->tex.y - v1->tex.y) * s->theight) / height;
    s->v = H3D_FP16(v1->tex.y * s->theight);
    s->dz = H3D_FP16(v2->pos.w - v1->pos.w) / height;
    s->z = H3D_FP16(v1->pos.w);
  }
  s->sectionheight = height;
  return height;
}

// Move to the next line along the side. Returns 1 if the
// entire side is done
static inline int _h3dtriside_next(_h3dtriside *s) {
  // At bottom of current section
  if (--s->sectionheight <= 0) {
    // There needs to be at least two vertices to work
    if (_h3dtriside_pop(s) < 2) {
      return 1;
    }
    // The next section has no height. Note that if this succeeds,
    // this begins the next section, and thus no recalcs are needed
    if (_h3dtriside_start(s) <= 0) {
      return 1;
    }
  } else {
    s->x += s->dx;
    if (s->trackall) {
      s->u += s->du;
      s->v += s->dv;
      s->z += s->dz;
    }
  }
  return 0;
}

void haloo3d_texturedtriangle_fast(haloo3d_fb *fb, haloo3d_fb *texture,
                                   mfloat_t intensity, haloo3d_facef face) {
  haloo3d_vertexf *v0v = face;
  haloo3d_vertexf *v1v = face + 1;
  haloo3d_vertexf *v2v = face + 2;
  haloo3d_vertexf *tmp;

  // NOTE: MAKE SURE YOU CLEAR THE Z-BUFFER TO A HIGH VALUE INSTEAD OF 0
  // Here, we fix v because it's actually the inverse
  v0v->tex.y = 1 - v0v->tex.y;
  v1v->tex.y = 1 - v1v->tex.y;
  v2v->tex.y = 1 - v2v->tex.y;

  // Make sure vertices are sorted top to bottom
  if (v0v->pos.y > v1v->pos.y) {
    tmp = v0v;
    v0v = v1v;
    v1v = tmp;
  }
  if (v1v->pos.y > v2v->pos.y) {
    tmp = v1v;
    v1v = v2v;
    v2v = tmp;
  }
  if (v0v->pos.y > v1v->pos.y) {
    tmp = v0v;
    v0v = v1v;
    v1v = tmp;
  }

  // Tracking info for left and right side. Doesn't track
  // every row; instead it tracks enough values to calulate the next
  _h3dtriside right, left;
  _h3dtriside_init(&right, texture);
  _h3dtriside_init(&left, texture);
  left.trackall = 1;
  _h3dtriside *onesec, *twosec;

  // Is this useful? I don't know...
  struct vec2i v0, v1, v2;
  vec2i_assign_vec2(v0.v, v0v->pos.v);
  vec2i_assign_vec2(v1.v, v1v->pos.v);
  vec2i_assign_vec2(v2.v, v2v->pos.v);

  mint_t parea = haloo3d_edgefunci(v0.v, v1.v, v2.v);
  if (parea == 0) {
    return;
  } else if (parea < 0) {
    // The middle point is on the right side, because it's wound clockwise
    onesec = &left;
    twosec = &right;
  } else {
    // The middle point is on the left side, because it's wound
    // counter-clockwise
    onesec = &right;
    twosec = &left;
  }
  _h3dtriside_push(twosec, v2v);
  _h3dtriside_push(twosec, v1v);
  _h3dtriside_push(twosec, v0v);
  _h3dtriside_push(onesec, v2v);
  _h3dtriside_push(onesec, v0v);
  // Calculate the deltas. If the "onesection" side has no height, we die
  if (_h3dtriside_start(onesec) <= 0) {
    return;
  }
  if (_h3dtriside_start(twosec) <= 0) {
    // The "twosection" side has another section. We skip the first if
    // it has nothing
    _h3dtriside_pop(twosec);
    if (_h3dtriside_start(twosec) <= 0) {
      return;
    }
  }

  const uint16_t scale = intensity * 256;

  uint16_t *buf_y = fb->buffer + v0.y * fb->width;
  mfloat_t *zbuf_y = fb->dbuffer + v0.y * fb->width;
  uint16_t *tbuf = texture->buffer;

  // NOTE ABOUT HOW THIS WORKS: the u and v are globally tracked with 16 bits.
  // but when going across spans, they are only tracked with 8 bits. This lets
  // us premultiply the v by width and have a constant right shift of 8 in the
  // loop. Apparently on x86, shifting by 8 is more optimized than 16; no idea
  // why
  const uint16_t twbits = log2(texture->width);
  const uint16_t tvshift = abs(8 - twbits);
  const int tvshleft = (twbits > 8);
  const uint16_t txr = texture->width - 1;
  const uint16_t tyr = (texture->height - 1) << twbits;

  // need to calc all the constant diffs
  const int32_t dzx = H3D_FP16(H3D_TRIDIFF_H(v0v, v1v, v2v, pos.w));
  const int32_t dux =
      H3D_FP16(H3D_TRIDIFF_H(v0v, v1v, v2v, tex.x) * texture->width) >> 8;
  int32_t dvx = H3D_FP16(H3D_TRIDIFF_H(v0v, v1v, v2v, tex.y) * texture->height);
  dvx = tvshleft ? (dvx << tvshift) : (dvx >> tvshift);

  while (1) {
    int xl = left.x >> 16;
    int xr = right.x >> 16;

    if (xl != xr) {
      uint16_t *buf = buf_y + xl;
      uint16_t *bufend = buf_y + xr;
      int32_t *zbuf = (int32_t *)(zbuf_y + xl);
      int32_t u = left.u >> 8;
      int32_t v = tvshleft ? (left.v << tvshift) : (left.v >> tvshift);
      int32_t z = left.z;

      do {
        if (z < *zbuf) {
          uint16_t c = tbuf[((u >> 8) & txr) + ((v >> 8) & tyr)];
          if (c & 0xF000) {
            *buf = haloo3d_col_scalei(c, scale);
            *zbuf = z;
          }
        }
        buf++;
        zbuf++;
        z += dzx;
        u += dux;
        v += dvx;
      } while (buf < bufend);
    }

    buf_y += fb->width;
    zbuf_y += fb->width;

    if (_h3dtriside_next(&left)) {
      return;
    }
    if (_h3dtriside_next(&right)) {
      return;
    }
  }
}

int haloo3d_facef_finalize(haloo3d_facef face) {
  // We HAVE to divide points first BEFORE checking the edge function
  haloo3d_vec4_conventional(&face[0].pos); // face[0].pos
  haloo3d_vec4_conventional(&face[1].pos); // face[0].pos
  haloo3d_vec4_conventional(&face[2].pos); // face[0].pos
  return haloo3d_edgefunc(face[0].pos.v, face[1].pos.v, face[2].pos.v) <= 0;
}

int haloo3d_facef_clip(haloo3d_facef face, haloo3d_facef *out) {
  // w + z (back)
  // w - z (front)
  // w + x (left)
  // w - x (right)
  // w + y (bottom)
  // w - y (top)

  int outers[3];
  int inners[3];
  mfloat_t dist[3];

  // We start with just the one face at index 0
  memcpy(out[0], face, sizeof(haloo3d_facef));

  // this is our "assignment" tracker. Bits are low to high.
  // We start with the first slot filled
  uint64_t assign = 1;

  // Do for each plane
  for (int p = 0; p < H3D_FACEF_CLIPPLANES; p++) {
    if (!assign) {
      break;
    }
    // How far to move to store triangle 2, also how many tris
    int tris = (1 << p);
    uint64_t tricheck = assign;
    // uint64_t mask = 1; // This will get left shifted
    for (int t = 0; t < tris; t++) {
      if (!tricheck) {
        break;
      }
      // Only do tris that are set
      if (tricheck & 1) {
        int numinners = 0;
        int numouters = 0;
        haloo3d_vertexf *f = out[t];
        // Figure out how many are in or out of this plane
        for (int i = 0; i < 3; i++) {
          switch (p) {
          case 0: // z-near
            dist[i] = f[i].pos.w + f[i].pos.z;
            break;
          // case 1: // z-far
          //   dist[i] = f[i].pos.w - f[i].pos.z;
          //   break;
          case 1: // x-left
            dist[i] = f[i].pos.w + f[i].pos.x;
            break;
          case 2: // x-right
            dist[i] = f[i].pos.w - f[i].pos.x;
            break;
          case 3: // y-bottom
            dist[i] = f[i].pos.w + f[i].pos.y;
            break;
          case 4: // t-top
            dist[i] = f[i].pos.w - f[i].pos.y;
            break;
          }
          if (dist[i] < H3D_FACEF_CLIPLOW) {
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
          mfloat_t tba = dist[bi] / (dist[bi] - dist[ai]) + H3D_FACEF_CLIPLOW;
          mfloat_t tca = dist[ci] / (dist[ci] - dist[ai]) + H3D_FACEF_CLIPLOW;
          // The two points that aren't 'a' need to be the interpolated values
          haloo3d_vertexf_lerp_self(f + bi, f + ai, tba);
          haloo3d_vertexf_lerp_self(f + ci, f + ai, tca);
          // Don't do anything with assign, it's already set for this tri
        } else if (numouters == 1) { // The two triangle thing
          // For this one, we need to mutate the original AND produce a new
          int ai = outers[0]; // A is the odd one out
          int bi = inners[0];
          int ci = inners[1];
          mfloat_t tab = dist[ai] / (dist[ai] - dist[bi]) + H3D_FACEF_CLIPLOW;
          mfloat_t tac = dist[ai] / (dist[ai] - dist[ci]) + H3D_FACEF_CLIPLOW;
          haloo3d_vertexf *f2 = out[t + tris];
          // BEFORE modification, we copy the existing triangle to the final
          // outer place
          memcpy(f2, f, sizeof(haloo3d_facef));
          assign |= (1L << (t + tris));
          // Fix existing triangle by replacing the bad outer point a
          // with an interpolated one to b
          haloo3d_vertexf_lerp_self(f + ai, f + bi, tab);
          haloo3d_vertexf olda = f[ai];
          // And once again replace the a point but interpolating with c
          haloo3d_vertexf_lerp_self(f2 + ai, f2 + ci, tac);
          // But the B point needs to actually be the interpolated A point
          f2[bi] = olda;
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

  while (assign) {
    if (assign & 1) {
      if (index != numout) {
        memcpy(out + numout, out + index, sizeof(haloo3d_facef));
      }
      numout++;
    }
    assign >>= 1;
    index++;
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
