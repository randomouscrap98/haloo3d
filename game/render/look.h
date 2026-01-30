#ifndef __RENDER_LOOK_H__
#define __RENDER_LOOK_H__

#include "../../haloo3d.h"

#define DEFAULTUP(x)    VEC3(x, 0, 1, 0)
#define DEFAULTLOOK(x)  VEC3(x, 0, 0, -1)
#define INIT_FOV        H3DVF(90.0)
#define INIT_NEARCLIP   H3DVF(0.010)
#define INIT_FARCLIP    H3DVF(100.0)

// Data describing position, orientation, and scale in a space. Useful for
// both models and cameras
typedef struct {
  vec3 pos;
  vec3 up;
  vec3 lookvec;
  vec3 scale; // Not exactly an "orientation" but useful
} render_orientation;

// Data describing the camera for rendering. Although you can set a "scale"
// on this, realistically it won't do anything.
typedef struct {
  hfloat_t fov;
  hfloat_t nearclip;
  hfloat_t farclip;
} render_camera;

void render_orientation_init(render_orientation * ro);
void render_orientation_matrix(render_orientation * ro, mat4 scr_matrix, mat4 out);

void render_camera_init(render_camera * rc);
void render_camera_matrix(render_camera * rc, render_orientation * ro, 
    h3d_fb * screen, mat4 out);
// NOthing to free in camera

#endif
