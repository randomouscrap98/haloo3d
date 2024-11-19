#include "../../haloo3d.h"
#include "../../haloo3d_ex.h"
#include "../../haloo3d_unigi.h"
#include "speedtest.h"

#define REPEAT 100000

// We just make some simple 2d triangle functions to test whether the
// testing framework even works

// Very simple flat triangle
void flattriangle(h3d_fb *buf, h3d_rasterface rf) {
  // This is a wrapper macro around several other macros. use it if you have
  // simple needs for your shader
  H3DTRI_EASY_BEGIN(rf, buf->width, buf->height, linpol, 0, bufi) {
    buf->buffer[bufi] = 0xFF00;
  }
  H3DTRI_SCAN_END(buf->width);
}

int main() {
  eprintf("Starting program\n");
  DEFAULTFB_UNIGI(fb);
  DEFAULT_RASTERFACE2D(rface);

  SPEEDTESTLOOP(flattriangle, REPEAT, fb, rface);

  h3d_fb_free(&fb);
  eprintf("Ending program\n");
}
