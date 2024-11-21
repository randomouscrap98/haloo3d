#include "../../haloo3d.h"
#include "../../haloo3d_ex.h"
#include "../../haloo3d_unigi.h"
#include "speedtest.h"

#define REPEAT 10000
#define TEXTURE "texture.ppm"

#define DITHEREND 2.2
#define DITHERSTART 1.0
#define DITHERSCALE 1.0 / (DITHEREND - DITHERSTART);

// Fix a rasterface for use in perspective correct things
#define PCRF(rfn, rfb)                                                         \
  h3d_rasterface rfn;                                                          \
  memcpy(rf, rfb, sizeof(h3d_rasterface));                                     \
  /* Persp correct is inverse z */                                             \
  for (int f = 0; f < 3; f++) {                                                \
    rf[f].interpolants[0] = 1.0 / rf[f].interpolants[0];                       \
    rf[f].interpolants[1] *= rf[f].interpolants[0];                            \
    rf[f].interpolants[2] *= rf[f].interpolants[0];                            \
  }

uint8_t _dither[] = H3D_DITHER4x4;

#define DITHERCALC(z)                                                          \
  if (!dithermask) {                                                           \
    dithermask = 1;                                                            \
    hfloat_t dnorm = (DITHEREND - (z)) * DITHERSCALE;                          \
    dither = _dither[(_y & 3) + h3d_4x4dither_index(dnorm)] & basedither;      \
  }

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
  // This might destroy the perf monitor, ugh (but it's
  // relatively accurate to what we have to do per-tri)
  PCRF(rf, rfb);
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
  // This might destroy the perf monitor, ugh (but it's
  // relatively accurate to what we have to do per-tri)
  PCRF(rf, rfb);
  H3DTRI_EASY_BEGIN(rf, buf->width, buf->height, linpol, 3, bufi) {
    hfloat_t z = 1 / linpol[0];
    hfloat_t u = linpol[1] * z;
    hfloat_t v = linpol[2] * z;
    buf->buffer[bufi] = H3D_FB_GETUV(&texuv, u, v);
    H3DTRI_LINPOL3(linpol);
  }
  H3DTRI_SCAN_END(buf->width);
}

// Flat dithering on flat color
void tri3d_flat_fd(h3d_fb *buf, h3d_rasterface rf) {
  int basedithofs = h3d_4x4dither_index(0.5);
  uint8_t dither;
  H3DTRI_CLAMP(rf, buf->width, buf->height);
  H3DTRI_BEGIN(rf, sv, parea);
  H3DTRI_SCAN_BEGIN(sv, parea, linpol, 0, buf->width, buf->height, bufi) {
    dither = _dither[basedithofs + (_y & 3)];
    dither = H3D_ROR8N(dither, _xl);
  }
  H3DTRI_SHADER(bufi) {
    if (dither & 1) {
      buf->buffer[bufi] = 0xFF00;
    }
    dither = H3D_ROR8(dither);
  }
  H3DTRI_SCAN_END(buf->width);
}

// Distance dithering on flat color
void tri3d_flat_ddpc(h3d_fb *buf, h3d_rasterface rfb) {
  PCRF(rf, rfb);
  int basedithofs = h3d_4x4dither_index(1.0);
  uint8_t dithermask;
  uint8_t dither;
  uint8_t basedither = 0xFF;
  H3DTRI_CLAMP(rf, buf->width, buf->height);
  H3DTRI_BEGIN(rf, sv, parea);
  H3DTRI_SCAN_BEGIN(sv, parea, linpol, 1, buf->width, buf->height, bufi) {
    dither = _dither[basedithofs + (_y & 3)];
    dithermask = 0;
    DITHERCALC(H3DVF(1) / linpol[0]);
    dithermask = 1 << (_xl & 7);
  }
  H3DTRI_SHADER(bufi) {
    hfloat_t pz = H3DVF(1) / linpol[0];
    DITHERCALC(pz);
    if (dither & dithermask) {
      buf->buffer[bufi] = 0xFF00;
    }
    dithermask <<= 1;
    H3DTRI_LINPOL1(linpol);
  }
  H3DTRI_SCAN_END(buf->width);
}

// Distance dithering on pc texture. This is close to what
// a real game would use
void tri3d_texuv_ddpc(h3d_fb *buf, h3d_rasterface rfb) {
  PCRF(rf, rfb);
  int basedithofs = h3d_4x4dither_index(1.0);
  uint8_t dithermask;
  uint8_t dither;
  uint8_t basedither = 0xFF;
  H3DTRI_CLAMP(rf, buf->width, buf->height);
  H3DTRI_BEGIN(rf, sv, parea);
  H3DTRI_SCAN_BEGIN(sv, parea, linpol, 3, buf->width, buf->height, bufi) {
    dither = _dither[basedithofs + (_y & 3)];
    dithermask = 0;
    DITHERCALC(H3DVF(1) / linpol[0]);
    dithermask = 1 << (_xl & 7);
  }
  H3DTRI_SHADER(bufi) {
    hfloat_t pz = H3DVF(1) / linpol[0];
    hfloat_t u = linpol[1] * pz;
    hfloat_t v = linpol[2] * pz;
    DITHERCALC(pz);
    if (dither & dithermask) {
      buf->buffer[bufi] = H3D_FB_GETUV(&texuv, u, v);
    }
    dithermask <<= 1;
    H3DTRI_LINPOL3(linpol);
  }
  H3DTRI_SCAN_END(buf->width);
}

static inline void calc3d_nomodel(h3d_rasterface out) {
  DEFAULT_RASTERFACE3D(rface);
  memcpy(out, rface, sizeof(h3d_rasterface));
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
  SPEEDTESTLOOP_GENERIC(calc3d_nomodel, REPEAT, rface);
  TEST(tri3d_flat);
  TEST(tri3d_vertexcolors);
  TEST(tri3d_vertexcolors_pc);
  TEST(tri3d_texuv);
  TEST(tri3d_texuv_pc);
  TEST(tri3d_flat_fd);
  TEST(tri3d_flat_ddpc);
  TEST(tri3d_texuv_ddpc);

  h3d_fb_free(&fb);
  h3d_fb_free(&texuv);
  eprintf("Ending program\n");
}
