#ifndef __RENDER_SCREEN_H__
#define __RENDER_SCREEN_H__

#include <stdlib.h>
#include "../../haloo3d.h"
#include "../../haloo3d_unigi.h"
#include "../unigi/main.h"
#include "../utils/log.h"

#define BACKTEX_MODE_TILED 1

typedef struct {
   h3d_fb window3d;     // 3d render frame, stored INSIDE here
   h3d_fb * screen;     // a reference to screen to render into after all 3d done
   h3d_fb * backtex;    // if set, fills window with given texture at given offset
   vec2i backtex_offset;
   uint8_t backtex_scale;
   uint8_t backtex_mode;
   // If alpha is 0, screen is not cleared
   uint16_t windowclear;
} render_3dscreen;

void render_3dscreen_init(render_3dscreen * render, h3d_fb * screen, uint8_t downscale);
void render_3dscreen_free(render_3dscreen * render);
void render_3dscreen_backfill(render_3dscreen * rs);

typedef struct {
  h3d_fb screen;
  h3d_easytimer blittimer;
  unigi_type_resolution resolution;
} render_window;

void render_window_init(render_window * render, int width, int height, char * windowname);
void render_window_free(render_window * render);
void render_window_blit(render_window * render);

#endif
