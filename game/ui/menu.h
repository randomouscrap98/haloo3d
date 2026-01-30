#ifndef __UI_MENU_H__
#define __UI_MENU_H__

#include "../../haloo3d.h"
#include "../render/timing.h"
#include "../render/screen.h"

#define PAUSEDARKEN 6

// Take full control away from the user and run a basic menu.
int menu_run_simple(const char * menu, int count, render_window * window, frametiming * ft);

#endif
