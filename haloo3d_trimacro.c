// Greatly reduce codegen by checking for impossible combinations.
// NOTE: we DO NOT output any errors if an invalid combination is present,
// we simply do not render the triangle!!
#if ((_HTF & (H3DR_TRANSPARENCY | H3DR_TEXTURED)) != H3DR_TRANSPARENCY) &&     \
    ((_HTF & (H3DR_DITHERTRI | H3DR_DITHERPIX)) !=                             \
     (H3DR_DITHERTRI | H3DR_DITHERPIX))

// Since we're including this file over and over, we have to undefine the stuff
// we're about to redefine. We do it like this because they could change based
// on _HTF, or the triangle flags
#undef H3D_DITHER_CHECK
#undef H3D_DITHERCALC
#undef H3D_TRANSPARENCY_CHECK
#undef H3D_SCALE_COL

// Dither can be quite expensive (sometimes), so it's a good savings to
// simply compile that stuff out
#if _HTF & H3DR_DITHERTRI
#define H3D_DITHER_CHECK(dither) (dither & 1)
#define H3D_DITHERCALC(z) /* do nothing */
#elif _HTF & H3DR_DITHERPIX
#define H3D_DITHER_CHECK(dither) (dither & dithermask)
#define H3D_DITHERCALC(z)                                                      \
  if (!dithermask) {                                                           \
    dithermask = 1;                                                            \
    /* NOTE: to scale light differently, you could do pow() on this */         \
    mfloat_t dnorm = (render->ditherfar - (z)) * ditherscale;                  \
    dither = render->ditherpattern[(y & 3) + haloo3d_4x4dither(dnorm)];        \
  }
#else
#define H3D_DITHER_CHECK(dither) 1
#define H3D_DITHERCALC(z) /* do nothing */
#endif

#if _HTF & H3DR_TRANSPARENCY
#define H3D_TRANSPARENCY_CHECK(c) if (c & 0xFF00)
#else
#define H3D_TRANSPARENCY_CHECK(c)
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
    mfloat_t ioz = left.ioz + xofs * dzx;
#if _HTF & H3DR_TEXTURED
    mfloat_t uoz = left.uoz + xofs * dux;
    mfloat_t voz = left.voz + xofs * dvx;
#endif
#else
    int32_t z = left.iz;
#if _HTF & H3DR_TEXTURED
    int32_t u = left.iu >> 8;
    int32_t v = tvshleft ? (left.iv << tvshift) : (left.iv >> tvshift);
#endif
#endif

#if _HTF & (H3DR_DITHERTRI)
    uint8_t dither = render->ditherpattern[dithofs + (y & 3)];
    dither = (dither >> (xl & 7)) | (dither << (8 - (xl & 7)));
#elif _HTF & (H3DR_DITHERPIX)
    uint8_t dithermask = 0;
    uint8_t dither;
#if _HTF & H3DR_PCT
    H3D_DITHERCALC(1 / ioz);
#else
    H3D_DITHERCALC(z >> 16);
#endif
    // MUST do this AFTER dithercalc since dithercalc sets dithermask
    dithermask = 1 << (xl & 7);
#endif

    do {
#if _HTF & H3DR_PCT
      // The horrible divide! Per pixel, no less!!
      mfloat_t pz = 1 / ioz;
      mfloat_t dpz = pz * 65536; // perspective incorrect is x16
      H3D_DITHERCALC(pz);
      if (dpz < *zbuf && H3D_DITHER_CHECK(dither)) {
#if _HTF & H3DR_TEXTURED
        uint16_t c = tbuf[(((uint32_t)(uoz * pz)) & txr) +
                          (((uint32_t)(voz * pz)) & tyr)];
#else
        uint16_t c = basecolor;
#endif
        H3D_TRANSPARENCY_CHECK(c) {
          *buf = H3D_SCALE_COL(c, scale);
          *zbuf = dpz;
        }
      }
#else
      H3D_DITHERCALC(z >> 16);
      if (z < *zbuf && H3D_DITHER_CHECK(dither)) {
#if _HTF & H3DR_TEXTURED
        uint16_t c = tbuf[((u >> 8) & txr) + ((v >> 8) & tyr)];
#else
        uint16_t c = basecolor;
#endif
        H3D_TRANSPARENCY_CHECK(c) {
          *buf = H3D_SCALE_COL(c, scale);
          *zbuf = z;
        }
      }
#endif
      buf++;
      zbuf++;
#if _HTF & H3DR_PCT
      ioz += dzx;
#if _HTF & H3DR_TEXTURED
      uoz += dux;
      voz += dvx;
#endif
#else
      z += dzxi;
#if _HTF & H3DR_TEXTURED
      u += duxi;
      v += dvxi;
#endif
#endif
#if _HTF & H3DR_DITHERTRI
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
#endif
