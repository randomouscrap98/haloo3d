#include "haloo3dex_easy.h"

void haloo3d_easy_calcdither4x4(haloo3d_trirender *settings, haloo3d_facef face,
                                mfloat_t ditherstart, mfloat_t ditherend) {
  mfloat_t avg = (face[0].pos.w + face[1].pos.w + face[2].pos.w) / 3;
  mfloat_t dither =
      (avg > ditherstart) ? (ditherend - avg) / (ditherend - ditherstart) : 1.0;
  haloo3d_trirender_setdither4x4(settings, dither);
}
