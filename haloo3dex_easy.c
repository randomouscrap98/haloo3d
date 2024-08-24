#include "haloo3dex_easy.h"
#include "haloo3d.h"
#include <string.h>

void haloo3d_easy_calcdither4x4(haloo3d_trirender *settings, haloo3d_facef face,
                                mfloat_t ditherstart, mfloat_t ditherend) {
  mfloat_t avg = (face[0].pos.w + face[1].pos.w + face[2].pos.w) / 3;
  mfloat_t dither =
      (avg > ditherstart) ? (ditherend - avg) / (ditherend - ditherstart) : 1.0;
  haloo3d_getdither4x4(dither, settings->dither);
}

void haloo3d_easystore_init(haloo3d_easystore *s) {
  for (int i = 0; i < H3D_EASYSTORE_MAX; i++) {
    s->objkeys[i][0] = 0;
    s->texkeys[i][0] = 0;
  }
}

#define _H3D_ES_CHECKKEY(key)                                                  \
  if (strlen(key) >= H3D_EASYSTORE_MAXKEY) {                                   \
    dieerr("Key too long! Max: %d\n", H3D_EASYSTORE_MAXKEY - 1);               \
  }
#define _H3D_ES_FOREACH(i) for (int i = 0; i < H3D_EASYSTORE_MAX; i++)
#define _H3D_ES_EMPTY(f) if (f[0] == 0)
#define _H3D_ES_NOTEMPTY(f) if (f[0] != 0)
#define _H3D_ES_FIND(f, key) if (strcmp(f, key) == 0)
#define _H3D_ES_NOEMPTY()                                                      \
  dieerr("No more room for objects! Max: %d\n", H3D_EASYSTORE_MAX);
#define _H3D_ES_NOFIND(key) dieerr("Object not found: %s\n", key);

haloo3d_obj *haloo3d_easystore_addobj(haloo3d_easystore *s, const char *key) {
  _H3D_ES_CHECKKEY(key);
  _H3D_ES_FOREACH(i) {
    _H3D_ES_EMPTY(s->objkeys[i]) {
      strcpy(s->objkeys[i], key);
      return s->_objects + i;
    }
  }
  _H3D_ES_NOEMPTY()
}

haloo3d_obj *haloo3d_easystore_getobj(haloo3d_easystore *s, const char *key) {
  _H3D_ES_CHECKKEY(key);
  _H3D_ES_FOREACH(i) {
    _H3D_ES_FIND(s->objkeys[i], key) { return s->_objects + i; }
  }
  _H3D_ES_NOFIND(key);
}

void haloo3d_easystore_deleteobj(haloo3d_easystore *s, const char *key,
                                 void (*ondelete)(haloo3d_obj *)) {
  _H3D_ES_CHECKKEY(key);
  _H3D_ES_FOREACH(i) {
    _H3D_ES_FIND(s->objkeys[i], key) {
      s->objkeys[i][0] = 0;
      if (ondelete != NULL) {
        ondelete(s->_objects + i);
      }
      return;
    }
  }
  _H3D_ES_NOFIND(key);
}

void haloo3d_easystore_deleteallobj(haloo3d_easystore *s,
                                    void (*ondelete)(haloo3d_obj *)) {
  _H3D_ES_FOREACH(i) {
    _H3D_ES_NOTEMPTY(s->objkeys[i]) {
      s->objkeys[i][0] = 0;
      if (ondelete != NULL) {
        ondelete(s->_objects + i);
      }
    }
  }
}

haloo3d_fb *haloo3d_easystore_addtex(haloo3d_easystore *s, const char *key) {
  _H3D_ES_CHECKKEY(key);
  _H3D_ES_FOREACH(i) {
    _H3D_ES_EMPTY(s->texkeys[i]) {
      strcpy(s->texkeys[i], key);
      return s->_textures + i;
    }
  }
  _H3D_ES_NOEMPTY()
}

haloo3d_fb *haloo3d_easystore_gettex(haloo3d_easystore *s, const char *key) {
  _H3D_ES_CHECKKEY(key);
  _H3D_ES_FOREACH(i) {
    _H3D_ES_FIND(s->texkeys[i], key) { return s->_textures + i; }
  }
  _H3D_ES_NOFIND(key);
}

void haloo3d_easystore_deletetex(haloo3d_easystore *s, const char *key,
                                 void (*ondelete)(haloo3d_fb *)) {
  _H3D_ES_CHECKKEY(key);
  _H3D_ES_FOREACH(i) {
    _H3D_ES_FIND(s->texkeys[i], key) {
      s->texkeys[i][0] = 0;
      if (ondelete != NULL) {
        ondelete(s->_textures + i);
      }
      return;
    }
  }
  _H3D_ES_NOFIND(key);
}

void haloo3d_easystore_deletealltex(haloo3d_easystore *s,
                                    void (*ondelete)(haloo3d_fb *)) {
  _H3D_ES_FOREACH(i) {
    _H3D_ES_NOTEMPTY(s->texkeys[i]) {
      s->texkeys[i][0] = 0;
      if (ondelete != NULL) {
        ondelete(s->_textures + i);
      }
    }
  }
}

void haloo3d_easytimer_init(haloo3d_easytimer *t, float avgweight) {
  t->sum = 0;
  t->last = 0;
  t->avgweight = avgweight;
}

void haloo3d_easytimer_start(haloo3d_easytimer *t) { t->start = clock(); }

void haloo3d_easytimer_end(haloo3d_easytimer *t) {
  clock_t end = clock();
  t->last = (float)(end - t->start) / CLOCKS_PER_SEC;
  if (t->sum == 0)
    t->sum = t->last;
  t->sum = t->avgweight * t->sum + (1 - t->avgweight) * t->last;
}

void haloo3d_easyrender_init(haloo3d_easyrender *r, int width, int height) {
  r->totalfaces = 0;
  r->totalverts = 0;
  r->nextobj = 0;
  r->autolightfix = 0;
  memset(r->_objstate, 0, sizeof(r->_objstate));
  haloo3d_trirender_init(&r->rendersettings);
  haloo3d_fb_init(&r->window, width, height);
  haloo3d_camera_init(&r->camera);
  haloo3d_print_initdefault(&r->tprint, r->printbuf, sizeof(r->printbuf));
  r->tprint.fb = &r->window;
  r->trifunc = H3D_EASYRENDER_NORMFUNC;
}

void haloo3d_easyrender_beginframe(haloo3d_easyrender *r) {
  haloo3d_print_refresh(&r->tprint);
  mfloat_t clearval;
  switch (r->trifunc) {
  case H3D_EASYRENDER_FASTFUNC:
    clearval = H3D_DBUF_FAST;
    break;
  case H3D_EASYRENDER_MIDFUNC:
    clearval = H3D_DBUF_MID;
    break;
  default:
    clearval = H3D_DBUF_NORM;
  }
  haloo3d_fb_cleardepth(&r->window, clearval);
  mfloat_t cammatrix[MAT4_SIZE];
  haloo3d_camera_calclook(&r->camera, cammatrix);
  mat4_inverse(cammatrix, cammatrix);
  mat4_multiply(r->screenmatrix, r->perspective, cammatrix);
}

void haloo3d_easyrender_beginmodel(haloo3d_easyrender *r,
                                   haloo3d_obj_instance *o) {
  mfloat_t tmp[VEC4_SIZE];
  mfloat_t modelm[MAT4_SIZE];
  mfloat_t finalmatrix[MAT4_SIZE];
  vec3_add(tmp, o->pos.v, o->lookvec.v);
  haloo3d_my_lookat(modelm, o->pos.v, tmp, o->up.v);
  // Apply scale such that it looks like it was applied first (this prevents
  // scaling applying skew to a rotated object)
  haloo3d_mat4_prescalev(modelm, o->scale.v);
  mat4_multiply(finalmatrix, r->screenmatrix, modelm);
  haloo3d_precalc_verts(o->model, finalmatrix, r->precalcs);
  r->rendersettings.texture = o->texture;
  if (r->autolightfix && o->lighting) {
    // Lighting doesn't rotate with the model unless you do it yourself.
    // In the easy system, you can request the renderer to do it for you
    struct vec4 ltmp, lout;
    // Lighting is centered at 0
    vec4(ltmp.v, 0, 0, 0, 1);
    // Calc the same lookat just without translation. THis should be the same
    // rotation matrix used on the model
    haloo3d_my_lookat(modelm, ltmp.v, o->lookvec.v, o->up.v);
    // We HAVE to have a vec4 (oof)
    vec4(ltmp.v, o->lighting->x, o->lighting->y, o->lighting->z, 1);
    haloo3d_vec4_multmat_into(&ltmp, modelm, &lout);
    // No need to fix W, should all be good (no perspective divide). But we DO
    // need to pull out that result
    vec3(r->fixedlighting.v, lout.x, lout.y, lout.z);
    vec3_normalize(r->fixedlighting.v, r->fixedlighting.v);
  }
}

haloo3d_obj_instance *haloo3d_easyrender_addinstance(haloo3d_easyrender *r,
                                                     haloo3d_obj *model,
                                                     haloo3d_fb *texture) {
  // For loop lets us finish if there's no space for new objects
  for (int i = 0; i < H3D_EASYRENDER_MAXOBJS; i++) {
    uint16_t thisobj = r->nextobj;
    r->nextobj = (r->nextobj + 1) % H3D_EASYRENDER_MAXOBJS;
    if (r->_objstate[thisobj] == 0) {
      r->_objstate[thisobj] = 1;
      r->totalfaces += model->numfaces;
      r->totalverts += model->numvertices;
      haloo3d_objin_init(r->objects + thisobj, model, texture);
      return r->objects + thisobj;
    }
  }
  dieerr("No more room for object instances! Max: %d\n",
         H3D_EASYRENDER_MAXOBJS);
}

// Delete the instance with the given pointer. Will not fail if it's not there.
void haloo3d_easyrender_deleteinstance(haloo3d_easyrender *r,
                                       haloo3d_obj_instance *in) {
  // no use doing anything if it's outside our buffer
  if (in < r->objects || in >= (r->objects + H3D_EASYRENDER_MAXOBJS)) {
    return;
  }
  // Deleting is just setting state to 0. The hole will get picked up
  // somewhere... also we track total faces and verts but who knows
  // if it'll be accurate. Users can add or delete faces and vertices
  // at runtime.
  uint16_t offset = in - r->objects;
  r->_objstate[offset] = 0;
  r->totalfaces -= r->objects[offset].model->numfaces;
  r->totalverts -= r->objects[offset].model->numvertices;
}

haloo3d_obj_instance *
haloo3d_easyrender_nextinstance(haloo3d_easyrender *r,
                                haloo3d_obj_instance *last) {
  if (last == NULL) {
    last = r->objects - 1; // Technically an invalid address but that's ok
  }

  while (last < r->objects + H3D_EASYRENDER_MAXOBJS - 1) {
    last++;
    if (r->_objstate[last - r->objects] != 0) {
      return last;
    }
  }

  return NULL;
}

int haloo3d_easyrender_renderface(haloo3d_easyrender *r,
                                  haloo3d_obj_instance *object, int facei,
                                  mfloat_t ditherstart, mfloat_t ditherend,
                                  mfloat_t minlight) {
  int totaldrawn = 0;
  haloo3d_facef face, baseface;
  // Copy face values out of precalc array and clip them
  haloo3d_make_facef(object->model->faces[facei], r->precalcs,
                     object->model->vtexture, face);
  int tris = haloo3d_facef_clip(face, r->outfaces);
  if (tris > 0) {
    if (ditherstart >= 0) {
      haloo3d_easy_calcdither4x4(&r->rendersettings, face, ditherstart,
                                 ditherend);
    }
    r->rendersettings.intensity = 1.0;
    if (object->lighting) {
      haloo3d_obj_facef(object->model, object->model->faces[facei], baseface);
      if (r->autolightfix) {
        r->rendersettings.intensity =
            haloo3d_calc_light(r->fixedlighting.v, minlight, baseface);
      } else {
        r->rendersettings.intensity =
            haloo3d_calc_light(object->lighting->v, minlight, baseface);
      }
    }
  }
  for (int ti = 0; ti < tris; ti++) {
    int backface = !haloo3d_facef_finalize(r->outfaces[ti]);
    if (object->cullbackface && backface) {
      continue;
    }
    totaldrawn++;
    //   We still have to convert the points into the view
    haloo3d_facef_viewport_into(r->outfaces[ti], r->window.width,
                                r->window.height);
    switch (r->trifunc) {
    case H3D_EASYRENDER_NORMFUNC:
      haloo3d_texturedtriangle(&r->window, &r->rendersettings, r->outfaces[ti]);
      break;
    case H3D_EASYRENDER_FASTFUNC:
      haloo3d_texturedtriangle_fast(&r->window, &r->rendersettings,
                                    r->outfaces[ti]);
      break;
    case H3D_EASYRENDER_MIDFUNC:
      haloo3d_texturedtriangle_mid(&r->window, &r->rendersettings,
                                   r->outfaces[ti]);
      break;
    }
  }
  return totaldrawn;
}

haloo3d_obj_instance *haloo3d_easyinstantiate(haloo3d_easyinstancer *ins,
                                              const char *name) {
  haloo3d_obj *obj = haloo3d_easystore_getobj(ins->storage, name);
  haloo3d_fb *tex = haloo3d_easystore_gettex(ins->storage, name);
  return haloo3d_easyrender_addinstance(ins->render, obj, tex);
}
