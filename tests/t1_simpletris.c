#include "../haloo3dex_helper.h"
#include "../haloo3dex_unigi.h"

// Simple solid triangle
void triangle(h3d_rastervertex *rv, h3d_fb *buf, uint16_t bw, uint16_t bh) {
  // This is a wrapper macro around several other macros. use it if you have
  // simple needs for your shader
  H3DTRI_EASY_BEGIN(rv, bw, bh, linpol, 0, bufi) { buf->buffer[bufi] = 0xFF00; }
  H3DTRI_SCAN_END();
}

void triangle_vcol(h3d_rastervertex *rv, h3d_fb *buf, uint16_t bw,
                   uint16_t bh) {
  H3DTRI_EASY_BEGIN(rv, bw, bh, linpol, 3, bufi) {
    buf->buffer[bufi] =
        H3DC_ARGB(15, (uint16_t)(15 * linpol[0]), (uint16_t)(15 * linpol[1]),
                  (uint16_t)(15 * linpol[2]));
    H3DTRI_LINPOL3(linpol);
  }
  H3DTRI_SCAN_END();
}

#define WIDTH 320
#define HEIGHT 240
#define OUTFILE "t1_simpletris_out.ppm"

int main() { // int argc, char **argv) {
  eprintf("Program start\n");
  // if (argc != 2) {
  //   dieerr("Must pass in a ppm file as texture to put on triangle!\n");
  // }

  // haloo3d_fb _tex;
  // haloo3d_img_loadppmfile(&_tex, argv[1]);
  h3d_fb screen;
  h3d_fb_init(&screen, WIDTH, HEIGHT);
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

  h3d_img_writeppmfile(&screen, OUTFILE);

  // h3d_fb_free(&_tex);
  h3d_fb_free(&screen);
}
