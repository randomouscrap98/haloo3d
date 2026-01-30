#ifndef __RENDER_TIMING_H__
#define __RENDER_TIMING_H__

#include "../../haloo3d_unigi.h"

#define FRAMETIMER_DEFAULTWEIGHT 0.85

typedef struct {
  hfloat_t delta_s;
  hfloat_t wait_margin;
  int target_fps;
  h3d_easytimer frame_timer;
  size_t frame_count;
} frametiming;

void frametiming_init(frametiming * timer, int target_fps);
// Call at the start of a frame
void frametiming_framestart(frametiming * timer);
// Call at the end of a frame
void frametiming_frameend(frametiming * timer);

void h3d_easytimer_init_unigi(h3d_easytimer * timer);

#endif
