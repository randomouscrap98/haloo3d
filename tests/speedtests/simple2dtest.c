// These speed tests test specifically unigi-formated buffers and
// how various 2d-only triangle functions may perform

#include "../../haloo3d.h"
#include "../../haloo3d_ex.h"
#include "../../haloo3d_unigi.h"
#include "speedtest.h"

#define REPEAT 10000
#define TEXTURE "texture.ppm"

// We just make some simple 2d triangle functions to test whether the
// testing framework even works

// Very simple flat triangle
void tri_flat(h3d_fb *buf, h3d_rasterface rf) {
  // This is a wrapper macro around several other macros. use it if you have
  // simple needs for your shader
  H3DTRI_EASY_BEGIN(rf, buf->width, buf->height, linpol, 0, bufi) {
    buf->buffer[bufi] = 0xFF00;
  }
  H3DTRI_SCAN_END(buf->width);
}

void tri_vertexcolors(h3d_fb *buf, h3d_rasterface rf) {
  // This is a wrapper macro around several other macros. use it if you have
  // simple needs for your shader
  H3DTRI_EASY_BEGIN(rf, buf->width, buf->height, linpol, 2, bufi) {
    uint8_t red = 15 * linpol[0];
    uint8_t green = 15 * linpol[1];
    buf->buffer[bufi] = H3DC_A4R4G4B4(15, red, green, 15);
    H3DTRI_LINPOL2(linpol);
  }
  H3DTRI_SCAN_END(buf->width);
}

// A gloabl for all textured triangles
h3d_fb texuv;

void tri_texuv(h3d_fb *buf, h3d_rasterface rf) {
  // This is a wrapper macro around several other macros. use it if you have
  // simple needs for your shader
  H3DTRI_EASY_BEGIN(rf, buf->width, buf->height, linpol, 2, bufi) {
    buf->buffer[bufi] = H3D_FB_GETUV(&texuv, linpol[0], linpol[1]);
    // H3DC_A4R4G4B4(15, red, green, 15);
    H3DTRI_LINPOL2(linpol);
  }
  H3DTRI_SCAN_END(buf->width);
}

int main() {
  eprintf("Starting program\n");
  DEFAULTFB_UNIGI(fb);
  DEFAULT_RASTERFACE2D(rface);

  h3d_fb_loadppmfile(&texuv, TEXTURE, h3d_fb_in_A4R4G4B4);

  // All tests follow the same format, so simplify it
#define TEST(func) SPEEDTESTLOOP(func, REPEAT, fb, rface);

  TEST(tri_flat);
  TEST(tri_flat);
  TEST(tri_flat);
  TEST(tri_vertexcolors);
  TEST(tri_texuv);

  h3d_fb_free(&fb);
  h3d_fb_free(&texuv);
  eprintf("Ending program\n");
}
