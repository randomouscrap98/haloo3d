#ifndef __HALOO3D_CONSOLE_H
#define __HALOO3D_CONSOLE_H
#include "haloo3d.h"

// Basic commands to support registering values by name
// and getting/setting them from the console
#define H3D_DC_MAXNAME 64
#define H3D_DC_MAXVALUES 1024
#define H3D_DC_PATHCHAR '/'
// #define H3D_DC_MAXLINE 1024

typedef struct {
  char names[H3D_DC_MAXVALUES][H3D_DC_MAXNAME];
  void *values[H3D_DC_MAXVALUES];
  int nextvalue;
} haloo3d_debugconsole;

// Initialize debug console so it's entirely empty
void haloo3d_debugconsole_init(haloo3d_debugconsole *dc);

// Get a value by name, return raw index. Returns -1 if not found
int haloo3d_debugconsole_get(haloo3d_debugconsole *dc, char *name);

// Add a value by name to the system or set an existing value.
// Relatively expensive; don't do this in critical loops! Returns -1
// if there's a problem, otherwise returns the index the value was added at
int haloo3d_debugconsole_set(haloo3d_debugconsole *dc, char *name, void *value);

// Delete a value. Return the original value. Returns NULL if not found
void *haloo3d_debugconsole_delete(haloo3d_debugconsole *dc, char *name);

// Begin a prompt session from stdin for get/set/etc. BLOCKING
void haloo3d_debugconsole_beginprompt(haloo3d_debugconsole *dc);
// List (to stdout) the folders and values at the level indicated by folder.
// Pass NULL to list the top level (empty string may not work)
void haloo3d_debugconsole_list(haloo3d_debugconsole *dc, char *folder);
// Given a name, return a pointer into the same name for the type. Since the
// type goes at the end, this should always be a valid string. The dot is NOT
// included
char *haloo3d_debugconsole_type(char *name);

#endif
