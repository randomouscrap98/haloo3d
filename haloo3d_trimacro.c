#undef H3D_DITHER_CHECK
#undef H3D_DITHERCALC
#undef H3D_TRANSPARENCY_CHECK
#undef H3D_SCALE_COL

#if _HTF & H3DR_DITHERTRI
#define H3D_DITHER_CHECK(dither) (dither & 1)
#define H3D_DITHERCALC(z) /* do nothing */
#elif _HTF & H3DR_DITHERPIX
#define H3D_DITHER_CHECK(dither) (dither & dithermask)
#define H3D_DITHERCALC(z)                                                      \
  if (!dithermask) {                                                           \
    dithermask = 1;                                                            \
    dither = render->ditherpattern[(y & 3) +                                   \
                                   haloo3d_4x4dither((render->ditherfar - z) * \
                                                     ditherscale)];            \
  }
#else
#define H3D_DITHER_CHECK(dither) 1
#define H3D_DITHERCALC(z) /* do nothing */
#endif

#if _HTF & H3DR_TRANSPARENCY
#define H3D_TRANSPARENCY_CHECK(c) (c & 0xFF00)
#else
#define H3D_TRANSPARENCY_CHECK(c) 1
#endif

#if _HTF & H3DR_LIGHTING
#define H3D_SCALE_COL(c, s) (haloo3d_col_scalei(c, s))
#else
#define H3D_SCALE_COL(c, s) (c)
#endif

while (1) {
#if _HTF & H3DR_PCT
  // Supposed to use ceiling but idk, mine works with floor... kinda.
  // I have holes but with ceil I get actual seams.
  int xl = left.x;
  int xr = right.x;
#else
  int xl = left.ix >> 16;
  int xr = right.ix >> 16;
#endif

  if (xl < xr) {
    uint16_t *buf = buf_y + xl;
    uint16_t *bufend = buf_y + xr;
    mfloat_t *zbuf = (zbuf_y + xl);

#if _HTF & H3DR_PCT
    mfloat_t xofs = xl - left.x;
    mfloat_t uoz = left.uoz + xofs * dux;
    mfloat_t voz = left.voz + xofs * dvx;
    mfloat_t ioz = left.ioz + xofs * dzx;
#else
    int32_t u = left.iu >> 8;
    int32_t v = tvshleft ? (left.iv << tvshift) : (left.iv >> tvshift);
    int32_t z = left.iz;
#endif

#if _HTF & (H3DR_DITHERTRI)
    uint8_t dither = render->ditherpattern[dithofs + (y & 3)];
    dither = (dither >> (xl & 7)) | (dither << (8 - (xl & 7)));
#elif _HTF & (H3DR_DITHERPIX)
    uint8_t dithermask = 1 << (xl & 7);
#if _HTF & H3DR_PCT
    mfloat_t pz = 1 / ioz;
#else
    mfloat_t pz = z >> 16;
#endif
    uint8_t dither =
        render->ditherpattern[(y & 3) +
                              haloo3d_4x4dither((render->ditherfar - pz) *
                                                ditherscale)];
#endif

    do {
#if _HTF & H3DR_PCT
      // The horrible divide! Per pixel, no less!!
      mfloat_t pz = 1 / ioz;
      mfloat_t dpz = pz * 65536; // perspective incorrect is x16
      H3D_DITHERCALC(pz);
      if (dpz < *zbuf && H3D_DITHER_CHECK(dither)) {
        uint16_t c = tbuf[(((uint32_t)(uoz * pz)) & txr) +
                          (((uint32_t)(voz * pz)) & tyr)];
        if (H3D_TRANSPARENCY_CHECK(c)) {
          *buf = H3D_SCALE_COL(c, scale);
          *zbuf = dpz;
        }
      }
#elif _HTF & H3DR_TEXTURED
      H3D_DITHERCALC(z);
      if (z < *zbuf && H3D_DITHER_CHECK(dither)) {
        uint16_t c = tbuf[((u >> 8) & txr) + ((v >> 8) & tyr)];
        if (H3D_TRANSPARENCY_CHECK(c)) {
          *buf = H3D_SCALE_COL(c, scale);
          *zbuf = z;
        }
      }
#else
      H3D_DITHERCALC(z);
      if (z < *zbuf && H3D_DITHER_CHECK(dither)) {
        *buf = H3D_SCALE_COL(basecolor, scale);
        *zbuf = z;
      }
#endif
      buf++;
      zbuf++;
#if _HTF & H3DR_PCT
      ioz += dzx;
      uoz += dux;
      voz += dvx;
#else
      u += duxi;
      v += dvxi;
      z += dzxi;
#endif
#if _HTF & (H3DR_DITHERTRI)
      dither = (dither >> 1) | (dither << 7);
#elif _HTF & H3DR_DITHERPIX
      dithermask <<= 1;
#endif
    } while (buf < bufend);
  }

  buf_y += fb->width;
  zbuf_y += fb->width;
  y++;

  if (nextfunc(&left)) {
    return;
  }
  if (nextfunc(&right)) {
    return;
  }
}
