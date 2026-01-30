#include "menu.h"
#include "../../haloo3d_ex.h"
#include "../render/effects.h"
#include "../unigi/main.h"
#include "../utils/keys.h"

int menu_run_simple(const char * menu, int count, render_window * window, frametiming * ft) {
  // Make a copy of the framebuffer as-is and darken it (for fun)
  h3d_fb background;
  h3d_fb_init(&background, window->screen.width, window->screen.height);
  memcpy(background.buffer, window->screen.buffer, 2 * H3D_FB_SIZE(&background));
  for(int i = 0; i < H3D_FB_SIZE(&background); i++) {
    EFFECT_DARKEN_A4R4G4B4(background.buffer[i], PAUSEDARKEN);
  }

  // REMEMBER TO FREE THE BACKGROUND!!
  logdebug("%s: %d", menu, count);

  while (1) {
    frametiming_framestart(ft);

    unigi_type_event event;
    do {
      unigi_event_get(&event);
      switch (event.type) {
      case unigi_enum_event_input_mouse_move:
        break;
      case unigi_enum_event_input_keyboard:
        if (event.data.input_keyboard.down) {
          switch (event.data.input_keyboard.button) {
          case KEY_ESCAPE:
            H3D_FB_FREE(&background);
            return -1;
          }
        }
        break;
      case unigi_enum_event_window_quit:
        exit(0);
        break;
      }
    } while (event.type != unigi_enum_event_none);

    // Dump the dark background directly in
    memcpy(window->screen.buffer, background.buffer, 2 * H3D_FB_SIZE(&background));

    render_window_blit(window);
    frametiming_frameend(ft);
  }

  H3D_FB_FREE(&background);
  return -1;
}
