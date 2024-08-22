// A bunch of helper functions for common tasks. They may not be
// highly configurable; use the core library functions if you need that
#ifndef __HALOO3D_EASY_H
#define __HALOO3D_EASY_H
#include "haloo3d.h"
#include "haloo3dex_print.h"
#include <time.h>

#define H3D_EASYRENDER_PRINTBUF 8192
#define H3D_EASYSTORE_MAX 1024
#define H3D_EASYSTORE_MAXKEY 16

#define H3D_EASYRENDER_MAXOBJS 1024
#define H3D_EASYRENDER_NORMFUNC 0
#define H3D_EASYRENDER_FASTFUNC 1

// A storage container for easy access to models and textures by name.
// Adds overhead compared to direct access of models and textures.
typedef struct {
  haloo3d_obj _objects[H3D_EASYSTORE_MAX];
  haloo3d_fb _textures[H3D_EASYSTORE_MAX];
  char objkeys[H3D_EASYSTORE_MAX][H3D_EASYSTORE_MAXKEY];
  char texkeys[H3D_EASYSTORE_MAX][H3D_EASYSTORE_MAXKEY];
} haloo3d_easystore;

// Initialize storage unit
void haloo3d_easystore_init(haloo3d_easystore *s);

haloo3d_obj *haloo3d_easystore_addobj(haloo3d_easystore *s, char *key);
haloo3d_obj *haloo3d_easystore_getobj(haloo3d_easystore *s, char *key);
void haloo3d_easystore_deleteobj(haloo3d_easystore *s, char *key,
                                 void (*ondelete)(haloo3d_obj *));
void haloo3d_easystore_deleteallobj(haloo3d_easystore *s,
                                    void (*ondelete)(haloo3d_obj *));

haloo3d_fb *haloo3d_easystore_addtex(haloo3d_easystore *s, char *key);
haloo3d_fb *haloo3d_easystore_gettex(haloo3d_easystore *s, char *key);
void haloo3d_easystore_deletetex(haloo3d_easystore *s, char *key,
                                 void (*ondelete)(haloo3d_fb *));
void haloo3d_easystore_deletealltex(haloo3d_easystore *s,
                                    void (*ondelete)(haloo3d_fb *));

// System for tracking time. Values measured in seconds. DON'T
// expect the 'start' variable to be of any particular type; it
// will depend on the implementation details
typedef struct {
  clock_t start;
  float sum;
  float last;
  float avgweight;
} haloo3d_easytimer;

void haloo3d_easytimer_init(haloo3d_easytimer *t, float avgweight);
void haloo3d_easytimer_start(haloo3d_easytimer *t);
void haloo3d_easytimer_end(haloo3d_easytimer *t);

// Storage for most of the rendering system
typedef struct {
  uint8_t _objstate[H3D_EASYRENDER_MAXOBJS];
  haloo3d_obj_instance objects[H3D_EASYRENDER_MAXOBJS];
  struct vec4 precalcs[H3D_OBJ_MAXVERTICES];
  haloo3d_facef outfaces[H3D_FACEF_MAXCLIP];
  char printbuf[H3D_EASYRENDER_PRINTBUF];
  mfloat_t perspective[MAT4_SIZE];
  mfloat_t screenmatrix[MAT4_SIZE];
  haloo3d_print_tracker tprint;
  haloo3d_camera camera;
  haloo3d_fb window;
  haloo3d_trirender rendersettings;
  uint32_t totalfaces;
  uint32_t totalverts;
  uint16_t nextobj;
  uint8_t trifunc;
} haloo3d_easyrender;

// Initialize ALL the values inside of the renderer
void haloo3d_easyrender_init(haloo3d_easyrender *r, int width, int height);
// initialize everything for the start of frame, like recalculating the camera
// matrix, clearing the depth buffer, and resetting the print cursor
void haloo3d_easyrender_beginframe(haloo3d_easyrender *r);
// calculate new vertices for rendering using renderer. MUST call beginframe
// first!
void haloo3d_easyrender_beginmodel(haloo3d_easyrender *r,
                                   haloo3d_obj_instance *o);
// Add a new object model instance, returning a pointer to the instance
haloo3d_obj_instance *haloo3d_easyrender_addinstance(haloo3d_easyrender *r,
                                                     haloo3d_obj *model,
                                                     haloo3d_fb *texture);
// Delete the instance with the given pointer. Will not fail if it's not there.
void haloo3d_easyrender_deleteinstance(haloo3d_easyrender *r,
                                       haloo3d_obj_instance *in);
// Continuously call this function to retrieve every single object. Pass NULL
// for the first instance.
haloo3d_obj_instance *
haloo3d_easyrender_nextinstance(haloo3d_easyrender *r,
                                haloo3d_obj_instance *last);
// Full render function using default values. If you need something special,
// such as unique dithering, you will need to do what this function does
// manually
int haloo3d_easyrender_renderface(haloo3d_easyrender *r,
                                  haloo3d_obj_instance *object, int facei,
                                  mfloat_t ditherstart, mfloat_t ditherend,
                                  mfloat_t minlight);

// Calculate the dither used for a face when your dither distance starts at
// start and ends at end. Also sets the dither on the render settings.
void haloo3d_easy_calcdither4x4(haloo3d_trirender *settings, haloo3d_facef face,
                                mfloat_t ditherstart, mfloat_t ditherend);

// Creating certain objects is a pain. If you put together a struct like this,
// you can call functions to easily create object instances where the name
// of the object and texture are the same, to simplify the process
typedef struct {
  haloo3d_easystore *storage;
  haloo3d_easyrender *render;
} haloo3d_easyinstancer;

// Quickly create an instance with texture/obj having the given name
haloo3d_obj_instance *haloo3d_easyinstantiate(haloo3d_easyinstancer *ins,
                                              char *name);

#endif
