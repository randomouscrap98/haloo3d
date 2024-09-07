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
// For efficiency, this function expects light to be NORMALIZED
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
  obj->color = 0xFFFF;
  obj->lighting = NULL; // Default no lighting
  vec3(obj->scale.v, 1.0, 1.0, 1.0);
  vec3(obj->pos.v, 0, 0, 0);
  // Assume user is going to use lookvec as a facing offset and not a raw
  // lookat. Though, this value will work regardless, considering the position
  vec3(obj->lookvec.v, 0, 0, -1);
  vec3(obj->up.v, 0, 1, 0);
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
  YAWP2VEC(cam->yaw, cam->pitch, lookvec.v);
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
  mat4_rotation_y(rot, -cam->yaw);
  struct vec4 result = haloo3d_vec4_multmat(delta, rot);
  *delta = result;
}

// ----------------------
//  Rendering
// ----------------------

// 4x4 dither patterns from 0 (no fill) to 16 (high fill). The pattern is
// actually 8x4 to make storage easier (each byte is 8 across)
// clang-format off
uint8_t _dither4x4[] = {
    0x00, 0x00, 0x00, 0x00,
    0x88, 0x00, 0x00, 0x00,
    0x88, 0x00, 0x22, 0x00,
    0xAA, 0x00, 0x22, 0x00,
    0xAA, 0x00, 0xAA, 0x00,
    0xAA, 0x44, 0xAA, 0x00,
    0xAA, 0x44, 0xAA, 0x11,
    0xAA, 0x55, 0xAA, 0x11,
    0xAA, 0x55, 0xAA, 0x55,
    0xEE, 0x55, 0xAA, 0x55,
    0xEE, 0x55, 0xBB, 0x55,
    0xFF, 0x55, 0xBB, 0x55,
    0xFF, 0x55, 0xFF, 0x55,
    0xFF, 0xDD, 0xFF, 0x55,
    0xFF, 0xDD, 0xFF, 0x77,
    0xFF, 0xFF, 0xFF, 0x77,
    0xFF, 0xFF, 0xFF, 0xFF,
};
// clang-format on

void haloo3d_trirender_init(haloo3d_trirender *tr) {
  tr->texture = NULL;
  tr->intensity = 1.0;
  // Dithering is off anyway but just in case...
  tr->ditherclose = 999999;
  tr->ditherfar = 999999;
  // Forces perspective correct textures
  tr->pctminsize = 0;
  tr->ditherpattern = _dither4x4;
  // Just set everything to true so the user's other settings all work
  tr->flags = H3DR_TRANSPARENCY | H3DR_LIGHTING | H3DR_TEXTURED | H3DR_PCT |
              H3DR_DITHERPIX;
}

static inline int haloo3d_4x4dither(float dither) {
  if (dither < 0) {
    return 0;
  } else if (dither > 1) {
    return 16 << 2;
  } else {
    return ((int)round(16 * dither) << 2);
  }
}

inline void haloo3d_getdither4x4(float dither, uint8_t *buf) {
  int index = haloo3d_4x4dither(dither);
  memcpy(buf, _dither4x4 + index, 4);
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
  mfloat_t x, uoz, voz, ioz;     // Tracking variables
  mfloat_t dx, duoz, dvoz, dioz; // Delta along CURRENT edge
  // These are 16.16 fixed point kinda
  int32_t ix, iu, iv, iz;     // Tracking variables (integer)
  int32_t idx, idu, idv, idz; // Delta along CURRENT edge (integer)
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
static inline int _h3dtriside_start_f(_h3dtriside *s) {
  const haloo3d_vertexf *const v1 = s->stack[s->top - 1];
  const haloo3d_vertexf *const v2 = s->stack[s->top - 2];
  // this won't throw away info; points are adjusted to whole nums
  const int height = v2->pos.y - v1->pos.y;
  if (height == 0) {
    return 0;
  }
  s->dx = (v2->pos.x - v1->pos.x) / height;
  s->x = v1->pos.x;
  if (s->trackall) {
    mfloat_t v2oz = 1 / v2->pos.w;
    mfloat_t v1oz = 1 / v1->pos.w;
    mfloat_t v2uoz = v2->tex.x * v2oz;
    mfloat_t v1uoz = v1->tex.x * v1oz;
    mfloat_t v2voz = v2->tex.y * v2oz;
    mfloat_t v1voz = v1->tex.y * v1oz;
    s->duoz = ((v2uoz - v1uoz) * s->twidth) / height;
    s->uoz = v1uoz * s->twidth;
    s->dvoz = ((v2voz - v1voz) * s->theight * s->twidth) / height;
    s->voz = v1voz * s->theight * s->twidth;
    s->dioz = (v2oz - v1oz) / height;
    s->ioz = v1oz;
  }
  s->sectionheight = height;
  return height;
}

// Move to the next line along the side. Returns 1 if the
// entire side is done
static inline int _h3dtriside_next_f(_h3dtriside *s) {
  // At bottom of current section
  if (--s->sectionheight <= 0) {
    // There needs to be at least two vertices to work
    if (_h3dtriside_pop(s) < 2) {
      return 1;
    }
    // The next section has no height. Note that if this succeeds,
    // this begins the next section, and thus no recalcs are needed
    if (_h3dtriside_start_f(s) <= 0) {
      return 1;
    }
  } else {
    s->x += s->dx;
    if (s->trackall) {
      s->uoz += s->duoz;
      s->voz += s->dvoz;
      s->ioz += s->dioz;
    }
  }
  return 0;
}

// Calculate all deltas (or at least all set to track) and return
// the height of this section.
static inline int _h3dtriside_start_i(_h3dtriside *s) {
  const haloo3d_vertexf *const v1 = s->stack[s->top - 1];
  const haloo3d_vertexf *const v2 = s->stack[s->top - 2];
  const int height =
      v2->pos.y - v1->pos.y; // this might throw away info, that's ok
  if (height == 0) {
    return 0;
  }
  s->idx = H3D_FP16(v2->pos.x - v1->pos.x) / height;
  s->ix = H3D_FP16(v1->pos.x);
  if (s->trackall) {
    s->idu = H3D_FP16((v2->tex.x - v1->tex.x) * s->twidth) / height;
    s->iu = H3D_FP16(v1->tex.x * s->twidth);
    s->idv = H3D_FP16((v2->tex.y - v1->tex.y) * s->theight) / height;
    s->iv = H3D_FP16(v1->tex.y * s->theight);
    s->idz = H3D_FP16(v2->pos.w - v1->pos.w) / height;
    s->iz = H3D_FP16(v1->pos.w);
  }
  s->sectionheight = height;
  return height;
}

// Move to the next line along the side. Returns 1 if the
// entire side is done
static inline int _h3dtriside_next_i(_h3dtriside *s) {
  // At bottom of current section
  if (--s->sectionheight <= 0) {
    // There needs to be at least two vertices to work
    if (_h3dtriside_pop(s) < 2) {
      return 1;
    }
    // The next section has no height. Note that if this succeeds,
    // this begins the next section, and thus no recalcs are needed
    if (_h3dtriside_start_i(s) <= 0) {
      return 1;
    }
  } else {
    s->ix += s->idx;
    if (s->trackall) {
      s->iu += s->idu;
      s->iv += s->idv;
      s->iz += s->idz;
    }
  }
  return 0;
}

// Main triangle drawing function
void haloo3d_triangle(haloo3d_fb *fb, haloo3d_trirender *render,
                      haloo3d_facef face) {
#ifdef H3DEBUG_SKIPWHOLETRI
  return;
#else
  haloo3d_vertexf *v0v = face;
  haloo3d_vertexf *v1v = face + 1;
  haloo3d_vertexf *v2v = face + 2;
  haloo3d_vertexf *tmp;

  uint8_t rflags = render->flags;

  if (rflags & (H3DR_DITHERPIX | H3DR_DITHERTRI)) {
    if (v0v->pos.w > render->ditherfar && v1v->pos.w > render->ditherfar &&
        v2v->pos.w > render->ditherfar) {
      // Trivially reject triangles that will not render because of dithering
      return;
    }
    if (v0v->pos.w < render->ditherclose && v1v->pos.w < render->ditherclose &&
        v2v->pos.w < render->ditherclose) {
      // Trivially remove expensive dithering for triangles that are fully
      // inside the dither start radius
      rflags &= ~(H3DR_DITHERPIX | H3DR_DITHERTRI);
    }
  }

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

#ifndef H3DEBUG_NOBOUNDSCHECK
  // We don't QUITE trust the triangles given, even though they should
  // be clipped. Just be safe and clamp them
  v0v->pos.x = CLAMP(v0v->pos.x, 0, fb->width - 1);
  v1v->pos.x = CLAMP(v1v->pos.x, 0, fb->width - 1);
  v2v->pos.x = CLAMP(v2v->pos.x, 0, fb->width - 1);
  v0v->pos.y = CLAMP(v0v->pos.y, 0, fb->height - 1);
  v1v->pos.y = CLAMP(v1v->pos.y, 0, fb->height - 1);
  v2v->pos.y = CLAMP(v2v->pos.y, 0, fb->height - 1);
#endif

  // Is this useful? I don't know...
  struct vec2i v0, v1, v2;
  vec2i_assign_vec2(v0.v, v0v->pos.v);
  vec2i_assign_vec2(v1.v, v1v->pos.v);
  vec2i_assign_vec2(v2.v, v2v->pos.v);

  // Tracking info for left and right side. Doesn't track
  // every row; instead it tracks enough values to calulate the next
  _h3dtriside right, left;
  _h3dtriside_init(&right, render->texture);
  _h3dtriside_init(&left, render->texture);
  left.trackall = 1;
  _h3dtriside *onesec, *twosec;

  mint_t parea = haloo3d_edgefunci(v0.v, v1.v, v2.v);
  if (parea == 0) {
    return;
  } else if (parea < 0) {
    // The middle point is on the right side, because it's wound clockwise
    parea = -parea;
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

  uint16_t basecolor = 0;
  if (v0v->tex.x == v1v->tex.x && v0v->tex.x == v2v->tex.x &&
      v0v->tex.y == v1v->tex.y && v0v->tex.y == v2v->tex.y) {
    // This is a single color triangle
    // No perspective or texture OR transparency needed
    rflags &= ~H3DR_TEXTURED;
  } else if (parea < render->pctminsize) {
    // This is a small triangle with textures
    // Turn off perspective correct textures
    rflags &= ~H3DR_PCT;
  }

  // More optimizations. No textures means no need for transparency or
  // perspective (also calculate base uv)
  if ((rflags & H3DR_TEXTURED) == 0) {
    rflags &= ~(H3DR_TRANSPARENCY | H3DR_PCT);
    basecolor = haloo3d_fb_getuv(render->texture, v0v->tex.x, v0v->tex.y);
  }

  int (*startfunc)(_h3dtriside *);
  int (*nextfunc)(_h3dtriside *);
  if (rflags & H3DR_PCT) {
    startfunc = _h3dtriside_start_f;
    nextfunc = _h3dtriside_next_f;
  } else {
    startfunc = _h3dtriside_start_i;
    nextfunc = _h3dtriside_next_i;
  }

  // Calculate the deltas. If the "onesection" side has no height, we die
  if (startfunc(onesec) <= 0) {
    return;
  }
  if (startfunc(twosec) <= 0) {
    // The "twosection" side has another section. We skip the first if
    // it has nothing
    _h3dtriside_pop(twosec);
    if (startfunc(twosec) <= 0) {
      return;
    }
  }

  uint16_t *buf_y = fb->buffer + v0.y * fb->width;
  mfloat_t *zbuf_y = fb->dbuffer + v0.y * fb->width;
  uint16_t *tbuf = render->texture->buffer;

  uint32_t txr = left.twidth - 1;
  uint32_t tyr = (left.theight - 1) * left.twidth;

  int tvshleft = 0;
  uint32_t tvshift = 0;
  mfloat_t dzx = 0, dux = 0, dvx = 0;
  int32_t dzxi = 0, duxi = 0, dvxi = 0;

  // need to calc all the constant horizontal diffs. The strides calculate
  // the vertical diffs.
  if (rflags & H3DR_PCT) {
    // Perspective correct values, which are all floating point and simple
    const mfloat_t v0ioz = 1 / v0v->pos.w;
    const mfloat_t v1ioz = 1 / v1v->pos.w;
    const mfloat_t v2ioz = 1 / v2v->pos.w;
    // clang-format off
    dzx = H3D_TRIDIFF_HG(v0v, v1v, v2v, v0ioz, v1ioz, v2ioz);
    dux = H3D_TRIDIFF_HG(v0v, v1v, v2v, v0v->tex.x * v0ioz, v1v->tex.x * v1ioz,
                         v2v->tex.x * v2ioz) * left.twidth;
    dvx = H3D_TRIDIFF_HG(v0v, v1v, v2v, v0v->tex.y * v0ioz, v1v->tex.y * v1ioz,
                         v2v->tex.y * v2ioz) * left.theight * left.twidth;
    // clang-format on
  } else {
    // NOTE ABOUT HOW THIS WORKS: the u and v are globally tracked with 16 bits.
    // but when going across spans, they are only tracked with 8 bits. This lets
    // us premultiply the v by width and have a constant right shift of 8 in the
    // loop. Apparently on x86, shifting by 8 is more optimized than 16; no idea
    // why
    const int twbits = log2(left.twidth);
    tvshift = abs(8 - twbits);
    tvshleft = (twbits > 8);
    // need to calc all the constant diffs
    dzxi = H3D_FP16(H3D_TRIDIFF_H(v0v, v1v, v2v, pos.w));
    duxi = H3D_FP16(H3D_TRIDIFF_H(v0v, v1v, v2v, tex.x) * left.twidth) >> 8;
    dvxi = H3D_FP16(H3D_TRIDIFF_H(v0v, v1v, v2v, tex.y) * left.theight);
    dvxi = tvshleft ? (dvxi << tvshift) : (dvxi >> tvshift);
  }

  int y = v0.y;

  uint16_t scale = 0;
  if (render->intensity >= 1.0) {
    rflags &= ~H3DR_LIGHTING;
  } else {
    scale = render->intensity * 256;
  }

  mfloat_t ditherscale = 1 / (render->ditherfar - render->ditherclose);
  int dithofs = 0;

  if (rflags & H3DR_DITHERTRI) {
    mfloat_t avg = (v0v->pos.w + v1v->pos.w + v2v->pos.w) / 3;
    dithofs = haloo3d_4x4dither((render->ditherfar - avg) * ditherscale);
  }

#ifndef H3DEBUG_SKIPTRIPIX
  // eprintf("RFLAGS: %d\n", rflags);
  switch (rflags) {
#include "haloo3d_trimacroswitch.c"
  default:
    dieerr("UNSUPPORTED TRI FLAG: %d", rflags);
  }
#endif
#endif
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

  // We start with just the one face at index 0
  memcpy(out[0], face, sizeof(haloo3d_facef));

#ifdef H3DEBUG_NOCLIPPING
  return 1;
#else
  int outers[3];
  int inners[3];
  mfloat_t dist[3];

  // this is our "assignment" tracker. Bits are low to high.
  // We start with the first slot filled
  uint64_t assign = 1;

  // Do for each plane
  for (int p = 0; p < H3D_FACEF_CLIPPLANES; p++) {
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

  // Though this could be expensive, generally speaking, there won't be
  // many triangles here. It doesn't scale well though
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
#endif
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
      if (pix & 0xF000) {
        haloo3d_fb_set(fb, x, y, pix);
      }
      texx += stepx;
    }
    texy += stepy;
  }
}

#define _H3D_FBF_BL(dbuf, sbuf)                                                \
  *dbuf = *sbuf;                                                               \
  dbuf++;

// Explicit loop unrolling for various amounts of scale factors
#define _H3D_FBF_ROW1(dbuf, sbuf, sbufe)                                       \
  while (sbuf < sbufe) {                                                       \
    _H3D_FBF_BL(dbuf, sbuf);                                                   \
    sbuf++;                                                                    \
  }

#define _H3D_FBF_ROW2(dbuf, sbuf, sbufe)                                       \
  while (sbuf < sbufe) {                                                       \
    _H3D_FBF_BL(dbuf, sbuf);                                                   \
    _H3D_FBF_BL(dbuf, sbuf);                                                   \
    sbuf++;                                                                    \
  }

#define _H3D_FBF_ROW3(dbuf, sbuf, sbufe)                                       \
  while (sbuf < sbufe) {                                                       \
    _H3D_FBF_BL(dbuf, sbuf);                                                   \
    _H3D_FBF_BL(dbuf, sbuf);                                                   \
    _H3D_FBF_BL(dbuf, sbuf);                                                   \
    sbuf++;                                                                    \
  }

#define _H3D_FBF_ROW4(dbuf, sbuf, sbufe)                                       \
  while (sbuf < sbufe) {                                                       \
    _H3D_FBF_BL(dbuf, sbuf);                                                   \
    _H3D_FBF_BL(dbuf, sbuf);                                                   \
    _H3D_FBF_BL(dbuf, sbuf);                                                   \
    _H3D_FBF_BL(dbuf, sbuf);                                                   \
    sbuf++;                                                                    \
  }

#define _H3D_FBF_ROWN(dbuf, sbuf, sbufe)                                       \
  while (sbuf < sbufe) {                                                       \
    for (int sx = 0; sx < scale; sx++) {                                       \
      _H3D_FBF_BL(dbuf, sbuf);                                                 \
    }                                                                          \
    sbuf++;                                                                    \
  }

void haloo3d_fb_fill(haloo3d_fb *dst, haloo3d_fb *src) {
  int scalex = dst->width / src->width;
  int scaley = dst->height / src->height;
  int scale = scalex < scaley ? scalex : scaley;
  if (scale == 0) {
    return;
  }
  int newwidth = scale * src->width;
  int newheight = scale * src->height;
  int dstofsx = (dst->width - newwidth) / 2;
  int dstofsy = (dst->height - newheight) / 2;
  // Need a step per y of src and a step per y of dst
  uint16_t *dbuf_y = &dst->buffer[dstofsx + dstofsy * dst->width];
  uint16_t *sbuf_y = src->buffer;
  uint16_t *sbuf_ye = src->buffer + src->width * src->height;
  // Iterate over original image
  while (sbuf_y < sbuf_ye) {
    for (int sy = 0; sy < scale; sy++) {
      uint16_t *sbuf = sbuf_y;
      uint16_t *sbufe = sbuf_y + src->width;
      uint16_t *dbuf = dbuf_y + dstofsx;
      switch (scale) {
      case 1:
        _H3D_FBF_ROW1(dbuf, sbuf, sbufe);
        break;
      case 2:
        _H3D_FBF_ROW2(dbuf, sbuf, sbufe);
        break;
      case 3:
        _H3D_FBF_ROW3(dbuf, sbuf, sbufe);
        break;
      case 4:
        _H3D_FBF_ROW4(dbuf, sbuf, sbufe);
        break;
      default:
        _H3D_FBF_ROWN(dbuf, sbuf, sbufe);
      }
      dbuf_y += dst->width;
    }
    sbuf_y += src->width;
  }
}
