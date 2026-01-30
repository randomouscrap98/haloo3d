#include "timing.h"
#include "../unigi/main.h"

static uint64_t _frametiming_gettime() {
  return unigi_time_get();
}

void h3d_easytimer_init_unigi(h3d_easytimer * timer) {
  h3d_easytimer_init(timer, FRAMETIMER_DEFAULTWEIGHT);
  timer->gettime = _frametiming_gettime;
  timer->timepersec = unigi_time_clocks_per_s;
}

void frametiming_init(frametiming * timer, int target_fps) {
  h3d_easytimer_init_unigi(&timer->frame_timer);
  timer->frame_count = 0;
  timer->target_fps = target_fps;
  timer->delta_s = 0.0f;
  timer->wait_margin = 1.0f / 1000.0f; // 1ms at least
}

void frametiming_framestart(frametiming * timer) {
  h3d_easytimer_start(&timer->frame_timer);
}

void frametiming_frameend(frametiming * timer) {
  h3d_easytimer_end(&timer->frame_timer);
  h3d_easytimer temp;
  h3d_easytimer_init_unigi(&temp);
  h3d_easytimer_start(&temp);
  // Wait for next frame based on fps
  float waittime = (1.0 / timer->target_fps) - timer->frame_timer.last;
  if (waittime > timer->wait_margin) {
    unigi_time_sleep(waittime * unigi_time_clocks_per_s);
  } 
  h3d_easytimer_end(&temp);
  timer->delta_s = timer->frame_timer.last + temp.last;
  timer->frame_count++;
}
