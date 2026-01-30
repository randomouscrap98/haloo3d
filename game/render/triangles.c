#include "triangles.h"
#include "../../haloo3d_unigi.h"

static triangle_properties _basic;

triangle_properties * triangle_properties_basic_texture(h3d_fb * texture) {
  _basic.texture = texture;
  _basic.flat_color = 0;
  return &_basic;
}

void render_environment_set_dithering(
  render_environment * r, hfloat_t dither_start, hfloat_t dither_end) {
  if(dither_end == dither_start) {
    dither_end++;
  }
  r->dither_end = dither_end;
  r->dither_start = dither_start;
  r->dither_scale = 1.0f / (r->dither_end - r->dither_start);
}

static const uint8_t _dither[] = H3D_DITHER4x4;

#define H3D_DITHERCALC(z, r)                                            \
  if (!dithermask) {                                                    \
    dithermask = 1;                                                     \
    hfloat_t dnorm = (r->dither_end - (z)) * r->dither_scale;           \
    dither = _dither[(_y & 3) + h3d_4x4dither_index(dnorm)];            \
  }

// draw a simple perspective correct textured triangle. First interpolant should
// be Z, second and third interpolant should be the texture u/v
void triangle_textured_pc(h3d_fb *buf, h3d_rasterface rf, h3d_fb texuv) {
  PCORRECT_TEXTURE_INTERPOLANT(rf);
  // int basedithofs = h3d_4x4dither_index(1.0);
  // uint8_t dithermask;
  // uint8_t dither;
  // uint8_t basedither = 0xFF;
  H3DTRI_CLAMP(rf, buf->width, buf->height);
  H3DTRI_BEGIN(rf, sv, parea);
  H3DTRI_SCAN_BEGIN(sv, parea, linpol, 3, buf->width, buf->height, bufi) {
  //   dither = _dither[basedithofs + (_y & 3)];
  //   dithermask = 0;
  //   DITHERCALC(H3DVF(1) / linpol[0]);
  //   dithermask = 1 << (_xl & 7);
  }
  H3DTRI_SHADER(bufi) {
    hfloat_t pz = H3DVF(1) / linpol[0];
    hfloat_t u = linpol[1] * pz;
    hfloat_t v = linpol[2] * pz;
    // DITHERCALC(pz);
    // if (dither & dithermask) {
    buf->buffer[bufi] = H3D_FB_GETUV(&texuv, u, v);
    // }
    // dithermask <<= 1;
    H3DTRI_LINPOL3(linpol);
  }
  H3DTRI_SCAN_END(buf->width);
}


// Rendering from the "maze" program. It can render faces that are either tectured
// or flat, with perspective correct textures. 3 interpolants, unless flat in which
// case only the first is updated (z)
void triangle_maze(const h3d_fb *fb, h3d_3dface f3, const triangle_properties * t,
               const render_environment * r) {
  if (f3[0].pos[H3DW] > r->dither_end && f3[1].pos[H3DW] > r->dither_end &&
      f3[2].pos[H3DW] > r->dither_end) {
    return; // trivially reject triangles outside the dither range
  }
  // Convert 3dface to rasterface for shader
  h3d_rasterface f;
  PCORRECT_FACE_INTERPOLANT(f3, fb, f);
  H3DTRI_CLAMP(f, fb->width, fb->height);
  H3DTRI_BEGIN(f, sv, parea);
  uint8_t dithermask;
  uint8_t dither;
  // NORMAL triangle draw has dithering applied, it's rather expensive...
  H3DTRI_SCAN_BEGIN(sv, parea, linpol, 3, fb->width, fb->height,
                    bufi) {
    // This is the per-row code
    dithermask = 0;
    H3D_DITHERCALC(1.0f / linpol[0], r);
    dithermask = 1 << (_xl & 7);
  }
  if (t->flat_color) {
    // Just for simplicity, jump into a different section during render
    // for flat objects
    H3DTRI_SHADER(bufi) {
      // This is the per-pixel code
      hfloat_t pz = 1.0f / linpol[0];
      H3D_DITHERCALC(pz, r);
      if ((dither & dithermask) && (linpol[0] > fb->dbuffer[bufi])) {
        fb->dbuffer[bufi] = linpol[0];
        fb->buffer[bufi] = t->flat_color;
      }
      H3DTRI_LINPOL1(linpol);
      dithermask <<= 1;
    }
  } else {
    H3DTRI_SHADER(bufi) {
      // This is the per-pixel code
      hfloat_t pz = H3DVF(1.0) / linpol[0];
      H3D_DITHERCALC(pz, r);
      if ((dither & dithermask) && (linpol[0] > fb->dbuffer[bufi])) {
        hfloat_t u = linpol[1] * pz;
        hfloat_t v = linpol[2] * pz;
        uint16_t col = H3D_FB_GETUV(t->texture, u, v);
        if (col & 0xF000) {
          fb->dbuffer[bufi] = linpol[0];
          fb->buffer[bufi] = col;
        }
      }
      H3DTRI_LINPOL3(linpol);
      dithermask <<= 1;
    }
  }
  H3DTRI_SCAN_END(fb->width);
}
