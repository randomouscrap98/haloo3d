#include "look.h"
#include "../../haloo3d_3d.h"

void render_orientation_init(render_orientation * ro) {
  DEFAULTUP(ro->up);
  DEFAULTLOOK(ro->lookvec);
  VEC3(ro->pos, 0, 0, 0);
  VEC3(ro->scale, 1.0f, 1.0f, 1.0f);
}

void render_orientation_matrix(render_orientation * ro, mat4 scr_matrix, mat4 out) {
  mat4 modelm;
  h3d_model_matrix(ro->pos, ro->lookvec, ro->up, ro->scale, modelm);
  mat4_multiply(scr_matrix, modelm, out);
}

void render_camera_init(render_camera * rc) {
  rc->nearclip = INIT_NEARCLIP;
  rc->farclip = INIT_FARCLIP;
  rc->fov = INIT_FOV;
}

// Precalculate the base screen matrix used to manipulate triangles into the 2d
// screen space, using the camera at the given render_placement
void render_camera_matrix(render_camera * rc, render_orientation * ro,
                          h3d_fb * screen, mat4 out) {
  vec3 lookat;
  mat4 cammatrix;
  mat4 perspective;
  // All we care about is that final "out" matrix, which we can use on models/tris
  h3d_perspective(rc->fov, (hfloat_t)screen->width / screen->height,
                  rc->nearclip, rc->farclip, perspective);
  vec3_add(ro->pos, ro->lookvec, lookat);
  h3d_my_lookat(ro->pos, lookat, ro->up, cammatrix);
  mat4_inverse(cammatrix, cammatrix);
  mat4_multiply(perspective, cammatrix, out);
}
