#include "../../haloo3d.h"
#include "../../haloo3d_ex.h"
#include "../../haloo3d_unigi.h"
#include "speedtest.h"

#define REPEAT 10000
#define TEXTURE "texture.ppm"

// We just make some simple 3d triangle functions to test whether the
// testing framework even works

// Very simple flat triangle
void tri3d_flat(h3d_fb *buf, h3d_rasterface rf) {
  H3DTRI_EASY_BEGIN(rf, buf->width, buf->height, linpol, 0, bufi) {
    buf->buffer[bufi] = 0xFF00;
  }
  H3DTRI_SCAN_END(buf->width);
}

void tri3d_vertexcolors(h3d_fb *buf, h3d_rasterface rf) {
  H3DTRI_EASY_BEGIN(rf, buf->width, buf->height, linpol, 3, bufi) {
    uint8_t red = 15 * linpol[1];
    uint8_t green = 15 * linpol[2];
    buf->buffer[bufi] = H3DC_A4R4G4B4(15, red, green, 15);
    H3DTRI_LINPOL3(linpol);
  }
  H3DTRI_SCAN_END(buf->width);
}

// Perspective correct vertex colors
void tri3d_vertexcolors_pc(h3d_fb *buf, h3d_rasterface rfb) {
  h3d_rasterface rf;
  memcpy(rf, rfb, sizeof(h3d_rasterface));
  // Persp correct is inverse z
  for (int f = 0; f < 3; f++) {
    rf[f].interpolants[0] = 1.0 / rf[f].interpolants[0];
    rf[f].interpolants[1] *= rf[f].interpolants[0];
    rf[f].interpolants[2] *= rf[f].interpolants[0];
  }
  H3DTRI_EASY_BEGIN(rf, buf->width, buf->height, linpol, 3, bufi) {
    hfloat_t z = 1 / linpol[0];
    uint8_t red = 15 * linpol[1] * z;
    uint8_t green = 15 * linpol[2] * z;
    buf->buffer[bufi] = H3DC_A4R4G4B4(15, red, green, 15);
    H3DTRI_LINPOL3(linpol);
  }
  H3DTRI_SCAN_END(buf->width);
}

// A gloabl for all textured triangles
h3d_fb texuv;

void tri3d_texuv(h3d_fb *buf, h3d_rasterface rf) {
  H3DTRI_EASY_BEGIN(rf, buf->width, buf->height, linpol, 3, bufi) {
    buf->buffer[bufi] = H3D_FB_GETUV(&texuv, linpol[1], linpol[2]);
    H3DTRI_LINPOL3(linpol);
  }
  H3DTRI_SCAN_END(buf->width);
}

void tri3d_texuv_pc(h3d_fb *buf, h3d_rasterface rfb) {
  // This might destroy the perf monitor, ugh
  h3d_rasterface rf;
  memcpy(rf, rfb, sizeof(h3d_rasterface));
  // Persp correct is inverse z
  for (int f = 0; f < 3; f++) {
    rf[f].interpolants[0] = 1.0 / rf[f].interpolants[0];
    rf[f].interpolants[1] *= rf[f].interpolants[0];
    rf[f].interpolants[2] *= rf[f].interpolants[0];
  }
  H3DTRI_EASY_BEGIN(rf, buf->width, buf->height, linpol, 3, bufi) {
    hfloat_t z = 1 / linpol[0];
    hfloat_t u = linpol[1] * z;
    hfloat_t v = linpol[2] * z;
    buf->buffer[bufi] = H3D_FB_GETUV(&texuv, u, v);
    H3DTRI_LINPOL3(linpol);
  }
  H3DTRI_SCAN_END(buf->width);
}

int main() {
  eprintf("Starting program\n");
  DEFAULTFB_UNIGI(fb);
  DEFAULT_RASTERFACE3D(rface);

  h3d_fb_loadppmfile(&texuv, TEXTURE);

  // All tests follow the same format, so simplify it
#define TEST(func) SPEEDTESTLOOP(func, REPEAT, fb, rface);

  TEST(tri3d_flat);
  TEST(tri3d_flat);
  TEST(tri3d_flat);
  TEST(tri3d_vertexcolors);
  TEST(tri3d_vertexcolors_pc);
  TEST(tri3d_texuv);
  TEST(tri3d_texuv_pc);

  h3d_fb_free(&fb);
  h3d_fb_free(&texuv);
  eprintf("Ending program\n");
}
