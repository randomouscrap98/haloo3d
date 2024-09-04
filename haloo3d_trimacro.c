#undef H3D_DITHER_CHECK
#undef H3D_TRANSPARENCY_CHECK
#undef H3D_SCALE_COL

#if _HTF & H3DR_DITHERTRI
#define H3D_DITHER_CHECK(dither) (dither & 1)
#else
#define H3D_DITHER_CHECK(dither) 1
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
    uint8_t dither = dithbuf[y & 3];
    dither = (dither >> (xl & 7)) | (dither << (8 - (xl & 7)));
#endif

    do {
#if _HTF & H3DR_PCT
      mfloat_t pz = 1 / ioz;
      if (pz < *zbuf && H3D_DITHER_CHECK(dither)) {
        // The horrible divide! Per pixel, no less!!
        uint16_t c = tbuf[(((uint32_t)(uoz * pz)) & txr) +
                          (((uint32_t)(voz * pz)) & tyr)];
        if (H3D_TRANSPARENCY_CHECK(c)) {
          *buf = H3D_SCALE_COL(c, scale);
          *zbuf = pz;
        }
      }
#else
      if (z < *zbuf && H3D_DITHER_CHECK(dither)) {
        uint16_t c = tbuf[((u >> 8) & txr) + ((v >> 8) & tyr)];
        if (H3D_TRANSPARENCY_CHECK(c)) {
          *buf = H3D_SCALE_COL(c, scale);
          *zbuf = z;
        }
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

//   while (1) {
//     int xl = left.x >> 16;
//     int xr = right.x >> 16;
//
//     if (xl != xr) {
//       uint16_t *buf = buf_y + xl;
//       uint16_t *bufend = buf_y + xr;
//       int32_t *zbuf = (int32_t *)(zbuf_y + xl);
//       int32_t u = left.u >> 8;
//       int32_t v = tvshleft ? (left.v << tvshift) : (left.v >> tvshift);
//       int32_t z = left.z;
// #ifndef H3D_FAST_NO_DITHERING
//       uint8_t dither = render->dither[y & 7];
//       dither = (dither >> (xl & 7)) | (dither << (8 - (xl & 7)));
// #endif
//
//       do {
//         if (z < *zbuf && H3D_DITHER_CHECK(dither)) {
//           uint16_t c = tbuf[((u >> 8) & txr) + ((v >> 8) & tyr)];
//           if (H3D_TRANSPARENCY_CHECK(c)) {
//             *buf = H3D_SCALE_COL(c, scale);
//             *zbuf = z;
//           }
//         }
//         buf++;
//         zbuf++;
//         z += dzx;
//         u += dux;
//         v += dvx;
// #ifndef H3D_FAST_NO_DITHERING
//         dither = (dither >> 1) | (dither << 7);
// #endif
//       } while (buf < bufend);
//     }
//
//     buf_y += fb->width;
//     zbuf_y += fb->width;
// #ifndef H3D_FAST_NO_DITHERING
//     y++;
// #endif
//
//     if (_h3dtriside_next(&left)) {
//       return;
//     }
//     if (_h3dtriside_next(&right)) {
//       return;
//     }
//   }
