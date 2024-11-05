#include "haloo3dex_console.h"
#include <string.h>

void haloo3d_debugconsole_init(haloo3d_debugconsole *dc) {
  dc->nextvalue = 0;
  for (int i = 0; i < H3D_DC_MAXVALUES; i++) {
    dc->names[i][0] = 0;
    dc->values[i] = NULL;
  }
}

int haloo3d_debugconsole_get(haloo3d_debugconsole *dc, char *name) {
  if (strlen(name) >= H3D_DC_MAXNAME) {
    eprintf("NAME TOO LARGE! Max: %d\n", H3D_DC_MAXNAME);
    return -1;
  }
  for (int i = 0; i < H3D_DC_MAXVALUES; i++) {
    if (strcmp(name, dc->names[i]) == 0) {
      return i;
    }
  }
  return -1;
}

int haloo3d_debugconsole_set(haloo3d_debugconsole *dc, char *name,
                             void *value) {
  if (strlen(name) >= H3D_DC_MAXNAME) {
    eprintf("NAME TOO LARGE! Max: %d\n", H3D_DC_MAXNAME);
    return -1;
  }
  // First, try to get the value. This will return NULL if there isn't one;
  // we don't want to add a new one if it already exists.
  int existing = haloo3d_debugconsole_get(dc, name);
  if (existing >= 0) {
    dc->values[existing] = value;
    return existing;
  }
  // We didn't find it, try to add it. Only iterate so much (iterator
  // is NOT the index, just a counter for retries)
  for (int i = 0; i < H3D_DC_MAXVALUES; i++) {
    int index = dc->nextvalue;
    dc->nextvalue = (dc->nextvalue + 1) % H3D_DC_MAXVALUES;
    if (dc->names[index][0] == 0) {
      // This is the place; add it and move on
      strcpy(dc->names[index], name);
      dc->values[index] = value;
      return index;
    }
  }
  eprintf("NO MORE ROOM FOR DEBUG VALUES! Max: %d\n", H3D_DC_MAXVALUES);
  return -1; // not found
}

char *haloo3d_debugconsole_type(char *name) {
  int end = strlen(name);
  while (end > 0) {
    end--; // Move back one
    // The first period from the end indicates the start of the extension
    if (name[end] == '.') {
      return name + end + 1;
    }
  }
  return name;
}

void *haloo3d_debugconsole_delete(haloo3d_debugconsole *dc, char *name) {
  if (strlen(name) >= H3D_DC_MAXNAME) {
    eprintf("NAME TOO LARGE! Max: %d\n", H3D_DC_MAXNAME);
    return NULL;
  }
  int existing = haloo3d_debugconsole_get(dc, name);
  if (existing >= 0) {
    // We found it, so we can delete it.
    dc->names[existing][0] = 0;
    void *value = dc->values[existing];
    dc->values[existing] = NULL;
    return value;
  }
  return NULL;
}

void haloo3d_debugconsole_list(haloo3d_debugconsole *dc, char *folder) {
  // For simplicity, use another debugconsole to track which folders we've seen.
  // This is wasteful but hopefully we don't blow the stack with this large
  // struct
  haloo3d_debugconsole printdc;
  haloo3d_debugconsole_init(&printdc);
  char thispath[H3D_DC_MAXNAME];
  int folderlength = 0;
  if (folder != NULL) {
    folderlength = strlen(folder);
    // if(folderlength && folder[folderlength - 1] == H3D_DC_PATHCHAR) {
    //   folderlength++;
    // }
  }
  for (int i = 0; i < H3D_DC_MAXVALUES; i++) {
    // Only look at ones that are active
    if (dc->names[i][0]) {
      // eprintf("Checking %s\n", dc->names[i]);
      // Only find things that match the string, if it exists
      if (folder == NULL || strstr(dc->names[i], folder) == dc->names[i]) {
        // Skip forward past the folder name, look for the first non / char
        char *inname = dc->names[i] + folderlength;
        while (*inname == H3D_DC_PATHCHAR) {
          inname++;
        }
        int len = 0;
        // We COULD be at the end of the string; this is OK. Look for the next /
        // char
        while (inname[len] != 0 && inname[len] != H3D_DC_PATHCHAR) {
          len++;
        }
        if (len) {
          if (inname[len] == H3D_DC_PATHCHAR) {
            len++;
          }
          // Copy out the string, null terminate it. We can be sure it's short
          // enough
          memcpy(thispath, inname, len);
          thispath[len] = 0; // There's a chance the len already includes 0
          // This automatically searches for an existing value, so repeats
          // should be trimmed.
          haloo3d_debugconsole_set(&printdc, thispath, (void *)thispath);
        }
      }
    }
  }
  // Now that we have our list, all we have to do is print it
  for (int i = 0; i < H3D_DC_MAXVALUES; i++) {
    if (printdc.names[i][0]) {
      printf("%s\n", printdc.names[i]);
    }
  }
}

void haloo3d_debugconsole_print(haloo3d_debugconsole *dc, char *path) {
  // Go find the index of the value
  int index = haloo3d_debugconsole_get(dc, path);
  if (index < 0) {
    printf("Can't find value %s\n", path);
  } else {
    char *type = haloo3d_debugconsole_type(dc->names[index]);
    void *v = dc->values[index];
    if (strcmp(type, "i") == 0) {
      printf("%s = %d\n", path, *(int *)v);
    } else if (strcmp(type, "u16x") == 0) {
      printf("%s = 0x%hx\n", path, *(uint16_t *)v);
    } else if (strcmp(type, "u8") == 0) {
      printf("%s = %hhu\n", path, *(uint8_t *)v);
    } else if (strcmp(type, "str") == 0) {
      printf("%s = %s\n", path, (char *)v);
    } else if (strcmp(type, "f") == 0) {
      printf("%s = %f\n", path, *(float *)v);
    } else {
      printf("Unsupported type: %s\n", type);
    }
  }
}

void haloo3d_debugconsole_beginprompt(haloo3d_debugconsole *dc) {
  printf("\n");
  char line[1024];
  char cmd[32];
  char path[1024]; // POTENTIAL ISSUE: line may be larger than etc etc
  char value[1024];
  while (1) {
    printf("DEBUG> ");
    char *fgetsresult = fgets(line, 1024, stdin);
    if (fgetsresult == NULL) {
      return;
    }
    // Scan for the most complicated format
    int values = sscanf(line, "%31s %1023s %1023s", cmd, path, value);
    if (values >= 1 && strcmp(cmd, "exit") == 0) {
      return;
    } else if (values >= 1 && strcmp(cmd, "help") == 0) {
      printf(" list [path]   - return all values and folders at path (none for "
             "root)\n");
      printf(" get key       - return value of key (full path)\n");
      printf(" set key value - set value of key (full path)\n");
    } else if (values >= 1 && strcmp(cmd, "list") == 0) {
      if (values == 1) {
        haloo3d_debugconsole_list(dc, NULL);
      } else {
        haloo3d_debugconsole_list(dc, path);
      }
    } else if (values == 2 && strcmp(cmd, "get") == 0) {
      haloo3d_debugconsole_print(dc, path);
    } else if (values == 3 && strcmp(cmd, "set") == 0) {
      // Go find the index of the value
      int index = haloo3d_debugconsole_get(dc, path);
      if (index < 0) {
        printf("Can't find value %s\n", path);
      } else {
        void *v = dc->values[index];
        char *type = haloo3d_debugconsole_type(path);
        int scanned = 0;
        if (strcmp(type, "i") == 0) {
          scanned = sscanf(value, "%d", (int *)v);
        } else if (strcmp(type, "u16") == 0) {
          scanned = sscanf(value, "%hu", (uint16_t *)v);
        } else if (strcmp(type, "u16x") == 0) {
          scanned = sscanf(value, "%hx", (uint16_t *)v);
        } else if (strcmp(type, "u8") == 0) {
          scanned = sscanf(value, "%hhu", (uint8_t *)v);
        } else if (strcmp(type, "f") == 0) {
          scanned = sscanf(value, "%f", (float *)v);
        } else if (strcmp(type, "str") == 0) {
          // This is EXTREMELY DANGEROUS: how big is that char container??
          // Is it even writable? I don't know... be careful!
          char *pathstr = strstr(line, path);
          // Get rid of the newline at the end before copying
          line[strlen(line) - 1] = 0;
          // Take everything past one past the end
          strcpy((char *)v, pathstr + strlen(path) + 1);
          scanned = 1;
        } else {
          printf("Unsupported type: %s\n", type);
          scanned = -1;
        }
        if (scanned == 0) {
          printf("Couldn't parse value into type %s\n", type);
        } else {
          haloo3d_debugconsole_print(dc, path);
        }
      }
    } else {
      printf("Unrecognized command\n");
    }
  }
}
