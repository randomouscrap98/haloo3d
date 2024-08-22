#include "../haloo3d.h"
#include "../haloo3dex_easy.h"
#include "../haloo3dex_gen.h"
#include "../haloo3dex_img.h"
#include "../haloo3dex_obj.h"
#include "../haloo3dex_print.h"
#include <stdlib.h>

#define WIDTH 640
#define HEIGHT 480
#define ASPECT ((float)WIDTH / HEIGHT)
#define FOV 60.0
#define NEARCLIP 0.01
#define FARCLIP 100.0

void renderframe(haloo3d_easyrender *render) {
  haloo3d_easyrender_beginframe(render);
  haloo3d_fb_clear(&render->window, 0xF000);
  haloo3d_obj_instance *object = NULL;
  int totaldrawn = 0;
  while ((object = haloo3d_easyrender_nextinstance(render, object)) != NULL) {
    //  Setup final model matrix and the precalced vertices
    haloo3d_easyrender_beginmodel(render, object);
    //  Iterate over object faces
    for (int fi = 0; fi < object->model->numfaces; fi++) {
      totaldrawn +=
          haloo3d_easyrender_renderface(render, object, fi, -1, -1, 0);
    }
  }
  eprintf("Drew %d triangles this frame\n", totaldrawn);
}

int main() {

  haloo3d_easystore storage;
  haloo3d_easystore_init(&storage);

  haloo3d_easyrender render;
  haloo3d_easyrender_init(&render, WIDTH, HEIGHT);
  render.trifunc = H3D_EASYRENDER_MIDFUNC;
  eprintf("Initialized renderer\n");

  haloo3d_obj *obj = haloo3d_easystore_addobj(&storage, "quad");
  haloo3d_fb *tex = haloo3d_easystore_addtex(&storage, "quad");

  struct vec3 center = {.x = 0, .y = 0, .z = 0};
  haloo3d_fb_init_tex(tex, 64, 64);
  haloo3d_gen_quad(obj, tex, center);

  // Fill texture with dithered color
  uint8_t dither[8];
  haloo3d_recti fill = {0, 0, tex->width, tex->height};
  haloo3d_getdither4x4(0.5, dither);
  haloo3d_apply_fillrect(tex, fill, 0xFF00, dither);

  // Print onto texture
  haloo3d_print_tracker pt;
  char buf[64];
  haloo3d_print_initdefault(&pt, buf, sizeof(buf));
  pt.fb = tex;
  pt.y = 28;
  haloo3d_print(&pt, " HELLO ");
  eprintf("Initialized models and textures\n");

  haloo3d_obj_instance *quadi =
      haloo3d_easyrender_addinstance(&render, obj, tex);
  vec3(quadi->scale.v, 0.3, 0.5,
       0.3); // Make it slightly taller than it should be
  quadi->cullbackface = 0;

  eprintf("Scene has %d tris, %d verts\n", render.totalfaces,
          render.totalverts);

  haloo3d_perspective(render.perspective, FOV, ASPECT, NEARCLIP, FARCLIP);

  render.camera.pos.z = 1;
  renderframe(&render);
  haloo3d_img_writeppmfile(&render.window, "quadlook_south.ppm");

  // Now rotate both the quad and the camera. Does something weird happen?
  render.camera.pos.z = 0;
  render.camera.pos.x = 1;
  render.camera.yaw = -MPI_2;
  // NOTE: the lookvec actually points along the same dir as the camera. so,
  // when the camera is looking in the -z dir, the quad is too, and what we see
  // is the backside? The quad might be set incorrectly then... not sure.
  // Semantics. The 1 would be normal if we were looking AT the camera, but
  // because we look ALONG with the camera, actually the backwards one is
  // forwards. This is probably bad but IDK what to do
  quadi->lookvec.x = 1;
  quadi->lookvec.z = 0;
  renderframe(&render);
  haloo3d_img_writeppmfile(&render.window, "quadlook_west.ppm");
  quadi->lookvec.x = -1;
  renderframe(&render);
  haloo3d_img_writeppmfile(&render.window, "quadlook_west_backwards.ppm");
  // Do something funny with up
  quadi->up.y = quadi->up.z = sin(MPI / 4);
  renderframe(&render);
  haloo3d_img_writeppmfile(&render.window,
                           "quadlook_west_backwards_rotate_half.ppm");
  quadi->up.y = 0;
  quadi->up.z = 1;
  renderframe(&render);
  haloo3d_img_writeppmfile(&render.window,
                           "quadlook_west_backwards_rotate_full.ppm");

  // And now, zoom WAY the hell out and set the scale larger
  quadi->up.y = 1;
  quadi->up.z = 0;
  vec3(quadi->scale.v, 3, 5, 3);
  render.camera.pos.x = 9;
  renderframe(&render);
  haloo3d_img_writeppmfile(&render.window, "quadlook_west_zoomout.ppm");

  haloo3d_easystore_deleteallobj(&storage, haloo3d_obj_free);
  haloo3d_easystore_deletealltex(&storage, haloo3d_fb_free);
}
