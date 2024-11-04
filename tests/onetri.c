#include "../haloo3dex_helper.h"
#include "../haloo3dex_unigi.h"

// void raster_triangle(h3d_rastervertex *vertices, uint8_t num_interpolants,
//  uint16_t bwidth, uint16_t bheight) {}
void triangle(h3d_rastervertex *rv, haloo3d_fb *buf, uint16_t bw, uint16_t bh) {
  H3DTRI_CLAMP(rv, bw, bh);
  H3DTRI_BEGIN(rv, sv, parea);
  H3DTRI_SCAN_BEGIN(sv, parea, linit, 0, bw, bh, bufi) {
    buf->buffer[bufi] = 0xFF00;
    // Don't need to call interpolants here, doing solid tri
  }
  H3DTRI_SCAN_END();
}

#define WIDTH 320
#define HEIGHT 240
#define OUTFILE "onetri_out.ppm"

int main(int argc, char **argv) {
  if (argc != 2) {
    dieerr("Must pass in a ppm file as texture to put on triangle!");
  }

  haloo3d_fb _tex;
  haloo3d_image_loadppmfile(&_tex, argv[1]);
  haloo3d_fb screen;
  haloo3d_fb_init(&screen, WIDTH, HEIGHT);

  haloo3d_image_writeppmfile(&screen, OUTFILE);

  haloo3d_fb_free(&_tex);
  haloo3d_fb_free(&screen);
}
