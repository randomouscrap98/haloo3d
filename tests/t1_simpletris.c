#include "../haloo3d.h"
#include "../haloo3d_ex.h"

// Simple solid triangle
void triangle(h3d_rastervert *rv, h3d_fb *buf) {
  // This is a wrapper macro around several other macros. use it if you have
  // simple needs for your shader.
  H3DTRI_EASY_BEGIN(rv, buf->width, buf->height, linpol, 0, bufi) {
    // Static color could be pulled outside the loop but compiler is smart
    buf->buffer[bufi] = H3DC_A1R5G5B5_F(1.0, 1.0, 0, 0);
  }
  H3DTRI_SCAN_END(buf->width);
}

void triangle_vcol(h3d_rastervert *rv, h3d_fb *buf) {
  H3DTRI_EASY_BEGIN(rv, buf->width, buf->height, linpol, 3, bufi) {
    // Interpolate vertex colors using linear interpolants
    buf->buffer[bufi] = H3DC_A1R5G5B5_F(1.0, linpol[0], linpol[1], linpol[2]);
    H3DTRI_LINPOL3(linpol);
  }
  H3DTRI_SCAN_END(buf->width);
}

#define WIDTH 320
#define HEIGHT 240
#define OUTFILE "t1_simpletris_out.ppm"

int main() {
  eprintf("Program start\n");

  h3d_fb screen;
  H3D_FB_INIT(&screen, WIDTH, HEIGHT, sizeof(uint16_t));
  eprintf("Initialized screen fb\n");

  h3d_rasterface rv;
  rv[0].pos[H3DX] = 20;
  rv[0].pos[H3DY] = 20;
  rv[1].pos[H3DX] = 20;
  rv[1].pos[H3DY] = 220;
  rv[2].pos[H3DX] = 270;
  rv[2].pos[H3DY] = 100;

  triangle(rv, &screen);
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

  triangle_vcol(rv, &screen);
  eprintf("Drew triangle 2\n");

  h3d_fb_writeppmfile(&screen, OUTFILE, h3d_fb_out_A1R5G5B5);

  H3D_FB_FREE(&screen);
}
