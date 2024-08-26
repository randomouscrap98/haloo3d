#ifndef __HALOO3D_ECS_H
#define __HALOO3D_ECS_H

#include "haloo3d.h"
#include "lib/mathc.h"

#define H3D_ECS_NORMFUNC 0
#define H3D_ECS_FASTFUNC 1
#define H3D_ECS_MIDFUNC 2

// Attach this to your player for their world position
// to be used as the view into the world. The player
// will then be the "renderer". It's up to you to provide
// all the pointers. If precalc_verts is null, the system
// will try to use the stack, though this may not be
// supported on all systems since the data is large.

typedef struct {
  haloo3d_fb *buffer;
  mfloat_t *perspective;
  // mfloat_t * precalc_screenmatrix; //[MAT4_SIZE];
  struct vec4 *precalc_verts; //[H3D_OBJ_MAXVERTICES];
  uint8_t trifunc;

  // haloo3d_facef outfaces[H3D_FACEF_MAXCLIP];
  // char printbuf[H3D_EASYRENDER_PRINTBUF];
  // mfloat_t perspective[MAT4_SIZE];
  // mfloat_t screenmatrix[MAT4_SIZE];
  // haloo3d_print_tracker tprint;
  // haloo3d_camera camera;
  // haloo3d_fb window;
  // haloo3d_trirender rendersettings;
  // uint32_t totalfaces;
  // uint32_t totalverts;
  // uint16_t nextobj;
  // uint8_t trifunc;
  // // Whether to automatically move lighting when models have a
  // // lookvec. This also normalizes the light
  // uint8_t autolightfix;
  // struct vec3 fixedlighting;
} h3decs_rendercontext;

// How to place and orient something in a "world"
typedef struct {
  struct vec3 pos;
  struct vec3 up;
  struct vec3 lookvec;
} h3decs_placement;

// Override the lookvec for placement with yaw/pitch
typedef struct vec2 h3decs_yawpitch;

// Attach this to your entity for it to become a render object. It holds
// render information per object
typedef struct {
  // 8x8 dithering (0 will make it transparent). Dithering is ANDed with
  // other per-triangle dithering to take the lowest of both.
  uint8_t dither[8];
  struct vec3 scale;
  struct vec3 *lighting; // a pointer to lighting, null for none
  uint16_t color;        // baseline color if textures aren't used
  uint8_t cullbackface;
  h3decs_rendercontext *context; // Which context to render into
} h3decs_renderobject;

// Attach this component to become the "view" into some
// world. You should only have one per context! When
// this entity runs, it will render everything.
// typedef struct {
//   mfloat_t * perspective;
//   h3decs_context * context;
// } h3decs_view;

#endif
