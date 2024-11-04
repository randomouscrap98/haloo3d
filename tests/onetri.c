#include "../haloo3dex_helper.h"
#include "../haloo3dex_unigi.h"

// void raster_triangle(h3d_rastervertex *vertices, uint8_t num_interpolants,
//  uint16_t bwidth, uint16_t bheight) {}
void triangle(h3d_rastervertex *rv, haloo3d_fb *buf, uint16_t bw, uint16_t bh) {
  H3DTRI_CLAMP(rv, bw, bh);
  H3DTRI_BEGIN(rv, sv, parea);
  eprintf("TRIANGLE READY: %d\n", parea);
  // Do calculations
  H3DTRI_SCAN_BEGIN(sv, parea, linit, 0, bw, bh, bufi) {
    buf->buffer[bufi] = 0xFF00;
    // Don't need to call interpolants here, doing solid tri
    (void)linit;
  }
  H3DTRI_SCAN_END();
}

void triangle_vcol(h3d_rastervertex *rv, haloo3d_fb *buf, uint16_t bw,
                   uint16_t bh) {
  H3DTRI_CLAMP(rv, bw, bh);
  H3DTRI_BEGIN(rv, sv, parea);
  // Do calculations
  H3DTRI_SCAN_BEGIN(sv, parea, linit, 3, bw, bh, bufi) {
    buf->buffer[bufi] =
        H3DC_ARGB(15, (uint16_t)(15 * linit[0]), (uint16_t)(15 * linit[1]),
                  (uint16_t)(15 * linit[2]));
    H3DTRI_LINIT3(linit);
  }
  H3DTRI_SCAN_END();
}

#define WIDTH 320
#define HEIGHT 240
#define OUTFILE "onetri_out.ppm"

int main() { // int argc, char **argv) {
  eprintf("Program start\n");
  // if (argc != 2) {
  //   dieerr("Must pass in a ppm file as texture to put on triangle!\n");
  // }

  // haloo3d_fb _tex;
  // haloo3d_img_loadppmfile(&_tex, argv[1]);
  haloo3d_fb screen;
  haloo3d_fb_init(&screen, WIDTH, HEIGHT);
  eprintf("Initialized screen fb\n");

  h3d_rastervertex rv[3];
  rv[0].pos[H3DX] = 20;
  rv[0].pos[H3DY] = 20;
  rv[1].pos[H3DX] = 20;
  rv[1].pos[H3DY] = 220;
  rv[2].pos[H3DX] = 270;
  rv[2].pos[H3DY] = 100;

  triangle(rv, &screen, WIDTH, HEIGHT);
  eprintf("Drew triangle 1\n");

  // Move some of the points over
  rv[0].pos[H3DX] = 150;
  rv[0].pos[H3DY] = 0;
  rv[2].pos[H3DX] = 200;
  rv[2].pos[H3DY] = 240;

  // Set up vertex colors
  rv[0].interpolants[0] = 1.0; // red
  rv[0].interpolants[1] = 0;
  rv[0].interpolants[2] = 0;
  rv[1].interpolants[0] = 0; // green
  rv[1].interpolants[1] = 1.0;
  rv[1].interpolants[2] = 0;
  rv[2].interpolants[0] = 0; // blue
  rv[2].interpolants[1] = 0;
  rv[2].interpolants[2] = 1.0;
  triangle_vcol(rv, &screen, WIDTH, HEIGHT);

  haloo3d_img_writeppmfile(&screen, OUTFILE);

  // haloo3d_fb_free(&_tex);
  haloo3d_fb_free(&screen);
}
