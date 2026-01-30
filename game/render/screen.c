#include "screen.h"
#include "timing.h"
#include "../../haloo3d_unigi.h"
#include "../../haloo3d_ex.h"


// Initialize the main render context (or any) to draw to default
// screen with default fov etc.
void render_3dscreen_init(render_3dscreen * render, h3d_fb * screen, uint8_t downscale) {
  render->screen = screen;
  render->backtex = NULL;
  render->backtex_scale = 1;
  render->backtex_mode = 0;
  VEC2(render->backtex_offset, 0, 0);
  render->windowclear = 0xF07F;
  h3d_fb_init(&render->window3d, screen->width / downscale, screen->height / downscale);
}

void render_3dscreen_free(render_3dscreen * render) {
  h3d_fb_free(&render->window3d);
}

// Essentially refresh 3d screen with background color or texture so you can start 
// drawing on top of it
void render_3dscreen_backfill(render_3dscreen * rs) {
  H3D_FB_DFILL(&rs->window3d, 0); // This is the DEPTH buffer!!!
  if (rs->windowclear & 0xF000) {
    H3D_FB_FILL(&rs->window3d, rs->windowclear);
  } 
  if (rs->backtex) {
    h3d_recti texrect;
    texrect.x1 = 0;
    texrect.y1 = 0;
    texrect.x2 = rs->backtex->width;
    texrect.y2 = rs->backtex->height;
    int nwidth = rs->backtex->width * rs->backtex_scale;
    int nheight = rs->backtex->height * rs->backtex_scale;
    h3d_recti outrect;
    outrect.x1 = rs->backtex_offset[H3DX];
    outrect.y1 = rs->backtex_offset[H3DY];
    outrect.x2 = outrect.x1 + nwidth;
    outrect.y2 = outrect.y1 + nheight;
    if(rs->backtex_mode == BACKTEX_MODE_TILED) {
      int startx = (rs->backtex_offset[H3DX] % nwidth) - nwidth;
      int starty = (rs->backtex_offset[H3DY] % nheight) - nheight;
      for(int y = starty; y < rs->window3d.height; y += nheight) {
        for(int x = startx; x < rs->window3d.width; x += nwidth) {
          outrect.x1 = x;
          outrect.y1 = y;
          outrect.x2 = outrect.x1 + nwidth;
          outrect.y2 = outrect.y1 + nheight;
          h3d_sprite(&rs->window3d, rs->backtex, texrect, outrect);
        }
      }
    } else {
      h3d_sprite(&rs->window3d, rs->backtex, texrect, outrect);
    }
  }
}

void render_window_init(render_window * render, int width, int height, char * windowname) {
  render->resolution.width = width;
  render->resolution.height = height;
  render->resolution.depth = 0;

  unigi_graphics_init();
  unigi_window_create(render->resolution, render->resolution, windowname);
  logdebug("Unigi system init: %d x %d(%s)", width, height, windowname);

  h3d_easytimer_init_unigi(&render->blittimer);
  h3d_fb_init(&render->screen, width, height);
}

void render_window_free(render_window * render) {
  H3D_FB_FREE(&render->screen);
}

void render_window_blit(render_window * render) {
  h3d_easytimer_start(&render->blittimer);
  unigi_graphics_blit(0, (unigi_type_color *)render->screen.buffer,
                      render->resolution.width * render->resolution.height);
  unigi_graphics_flush();
  h3d_easytimer_end(&render->blittimer);
}
