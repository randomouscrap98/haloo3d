#undef H3D_DITHER_CHECK
#undef H3D_TRANSPARENCY_CHECK
#undef H3D_SCALE_COL

#if _HTF & H3DR_DITHERTRI
#define H3D_DITHER_CHECK(dither, z) (dither & 1)
#elif _HTF & H3DR_DITHERPIX
#define H3D_DITHER_CHECK(dither, z)                                            \
  (render->ditherpattern[dithofs + haloo3d_4x4dither((render->ditherfar - z) * \
                                                     ditherscale)] &           \
   dither)
#else
#define H3D_DITHER_CHECK(dither, z) 1
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
    // This is a DITHER MASK, not the dither itself!
    uint8_t dither = 1 << (xl & 7);
    // Reuse dithofs (which is only used for H3DR_DITHERTRI) for y ofs (very
    // small savings)
    dithofs = y & 3;
#endif

    do {
#if _HTF & H3DR_PCT
      mfloat_t pz = 1 / ioz;
      if (pz < *zbuf && H3D_DITHER_CHECK(dither, pz)) {
        // The horrible divide! Per pixel, no less!!
        uint16_t c = tbuf[(((uint32_t)(uoz * pz)) & txr) +
                          (((uint32_t)(voz * pz)) & tyr)];
        if (H3D_TRANSPARENCY_CHECK(c)) {
          *buf = H3D_SCALE_COL(c, scale);
          *zbuf = pz;
        }
      }
#elif _HTF & H3DR_TEXTURED
      if (z < *zbuf && H3D_DITHER_CHECK(dither, z)) {
        uint16_t c = tbuf[((u >> 8) & txr) + ((v >> 8) & tyr)];
        if (H3D_TRANSPARENCY_CHECK(c)) {
          *buf = H3D_SCALE_COL(c, scale);
          *zbuf = z;
        }
      }
#else
      if (z < *zbuf && H3D_DITHER_CHECK(dither, z)) {
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
      dither = (dither << 1) | (dither >> 7);
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
