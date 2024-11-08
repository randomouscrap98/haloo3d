#include "haloo3d_unigi.h"
#include "haloo3d_ex.h"

#include <stdio.h>
#include <string.h>

// ===========================================
// |                 IMAGE                   |
// ===========================================

void h3d_fb_writeppm(h3d_fb *fb, FILE *f) {
  fprintf(f, "P6 %d %d 15\n", fb->width, fb->height);
  uint8_t color[3];
  for (int i = 0; i < H3D_FB_SIZE(fb); i++) {
    uint16_t bc = fb->buffer[i];
    color[0] = (bc >> 8) & 0xF; // H3DC_R(bc);
    color[1] = (bc >> 4) & 0xf; // H3DC_G(bc);
    color[2] = bc & 0xf;        // H3DC_B(bc);
    fwrite(color, sizeof(uint8_t), 3, f);
  }
}

void h3d_fb_loadppm(FILE *f, h3d_fb *fb) {
  char tmp[4096];
  // Must ALWAYS start with "P6"
  int scanned = fscanf(f, "%4095s", tmp);
  if (scanned != 1 || strcmp(tmp, "P6") != 0) {
    dieerr("Image file not in P6 format (no P6 identifier)");
  }
  // Now just pull three digits
  int vals[3];
  int numvals = 0;
  while (numvals != 3) {
    scanned = fscanf(f, "%d", vals + numvals);
    if (scanned != 1) {
      // This might just be a comment. Consume the rest of the line if so
      scanned = fscanf(f, "%4095s", tmp);
      if (scanned != 1 || tmp[0] != '#' || !fgets(tmp, 4095, f)) {
        dieerr("Image file not in P6 format (unexpected header value: %s)",
               tmp);
      }
    } else {
      numvals++;
    }
  }
  // Consume one character, it's the whitespace after depth
  fgetc(f);
  fb->width = vals[0];
  fb->height = vals[1];
  int depth = vals[2];
  H3D_FB_TEXINIT(fb, fb->width, fb->height);
  // Must set everything to 0
  memset(fb->buffer, 0, H3D_FB_SIZE(fb));
  // Now let's just read until the end!
  int b = 0;
  int i = 0;
  int c = 0;
  while ((c = fgetc(f)) != EOF) {
    fb->buffer[i] |=
        0xF000 | ((uint16_t)((c / (float)depth) * 15 + 0.5) << ((2 - b) * 4));
    b++;
    if (b == 3) { // We've read the full rgb
      i++;
      b = 0;
    }
  }
}

void h3d_fb_writeppmfile(h3d_fb *fb, char *filename) {
  // And now we should be able to save the framebuffer
  FILE *f = fopen(filename, "w");
  if (f == NULL) {
    dieerr("Can't open %s for writing ppm image\n", filename);
  }
  h3d_fb_writeppm(fb, f);
  fclose(f);
  eprintf("Wrote ppm image to %s\n", filename);
}

void h3d_fb_loadppmfile(h3d_fb *tex, char *filename) {
  // Open a simple file and read the ppm from it
  FILE *f = fopen(filename, "r");
  if (f == NULL) {
    dieerr("Can't open %s for ppm image reading\n", filename);
  }
  h3d_fb_loadppm(f, tex); // This also calls init so you have to free
  fclose(f);
  eprintf("Read ppm image from %s\n", filename);
}

// ===========================================
// |              FRAMEBUFFER                |
// ===========================================

void h3d_fb_init(h3d_fb *fb, uint16_t width, uint16_t height) {
  H3D_FB_INIT(fb, width, height, 2);
}

void h3d_fb_free(h3d_fb *fb) { H3D_FB_FREE(fb); }

// ===========================================
// |              EASYSYS                    |
// ===========================================

void h3d_easystore_init(h3d_easystore *s) {
  for (int i = 0; i < H3D_EASYSTORE_MAX; i++) {
    s->objkeys[i][0] = 0;
    s->texkeys[i][0] = 0;
  }
}

#define _H3D_ES_CHECKKEY(key)                                                  \
  if (strlen(key) >= H3D_EASYSTORE_MAXKEY) {                                   \
    dieerr("Key too long! Max: %d\n", H3D_EASYSTORE_MAXKEY - 1);               \
  }
#define _H3D_ES_FOREACH(i) for (int i = 0; i < H3D_EASYSTORE_MAX; i++)
#define _H3D_ES_EMPTY(f) if (f[0] == 0)
#define _H3D_ES_NOTEMPTY(f) if (f[0] != 0)
#define _H3D_ES_FIND(f, key) if (strcmp(f, key) == 0)
#define _H3D_ES_NOEMPTY()                                                      \
  dieerr("No more room for objects! Max: %d\n", H3D_EASYSTORE_MAX);
#define _H3D_ES_NOFIND(key) dieerr("Object not found: %s\n", key);

h3d_obj *h3d_easystore_addobj(h3d_easystore *s, const char *key) {
  _H3D_ES_CHECKKEY(key);
  _H3D_ES_FOREACH(i) {
    _H3D_ES_EMPTY(s->objkeys[i]) {
      strcpy(s->objkeys[i], key);
      return s->_objects + i;
    }
  }
  _H3D_ES_NOEMPTY()
}

h3d_obj *h3d_easystore_getobj(h3d_easystore *s, const char *key) {
  _H3D_ES_CHECKKEY(key);
  _H3D_ES_FOREACH(i) {
    _H3D_ES_FIND(s->objkeys[i], key) { return s->_objects + i; }
  }
  _H3D_ES_NOFIND(key);
}

void h3d_easystore_deleteobj(h3d_easystore *s, const char *key,
                             void (*ondelete)(h3d_obj *)) {
  _H3D_ES_CHECKKEY(key);
  _H3D_ES_FOREACH(i) {
    _H3D_ES_FIND(s->objkeys[i], key) {
      s->objkeys[i][0] = 0;
      if (ondelete != NULL) {
        ondelete(s->_objects + i);
      }
      return;
    }
  }
  _H3D_ES_NOFIND(key);
}

void h3d_easystore_deleteallobj(h3d_easystore *s, void (*ondelete)(h3d_obj *)) {
  _H3D_ES_FOREACH(i) {
    _H3D_ES_NOTEMPTY(s->objkeys[i]) {
      s->objkeys[i][0] = 0;
      if (ondelete != NULL) {
        ondelete(s->_objects + i);
      }
    }
  }
}

h3d_fb *h3d_easystore_addtex(h3d_easystore *s, const char *key) {
  _H3D_ES_CHECKKEY(key);
  _H3D_ES_FOREACH(i) {
    _H3D_ES_EMPTY(s->texkeys[i]) {
      strcpy(s->texkeys[i], key);
      return s->_textures + i;
    }
  }
  _H3D_ES_NOEMPTY()
}

h3d_fb *h3d_easystore_gettex(h3d_easystore *s, const char *key) {
  _H3D_ES_CHECKKEY(key);
  _H3D_ES_FOREACH(i) {
    _H3D_ES_FIND(s->texkeys[i], key) { return s->_textures + i; }
  }
  _H3D_ES_NOFIND(key);
}

void h3d_easystore_deletetex(h3d_easystore *s, const char *key,
                             void (*ondelete)(h3d_fb *)) {
  _H3D_ES_CHECKKEY(key);
  _H3D_ES_FOREACH(i) {
    _H3D_ES_FIND(s->texkeys[i], key) {
      s->texkeys[i][0] = 0;
      if (ondelete != NULL) {
        ondelete(s->_textures + i);
      }
      return;
    }
  }
  _H3D_ES_NOFIND(key);
}

void h3d_easystore_deletealltex(h3d_easystore *s, void (*ondelete)(h3d_fb *)) {
  _H3D_ES_FOREACH(i) {
    _H3D_ES_NOTEMPTY(s->texkeys[i]) {
      s->texkeys[i][0] = 0;
      if (ondelete != NULL) {
        ondelete(s->_textures + i);
      }
    }
  }
}

void h3d_easytimer_init(h3d_easytimer *t, float avgweight) {
  t->sum = 0;
  t->last = 0;
  t->avgweight = avgweight;
  t->min = 99999999.0;
  t->max = 0.0;
}

void h3d_easytimer_start(h3d_easytimer *t) { t->start = clock(); }

void h3d_easytimer_end(h3d_easytimer *t) {
  clock_t end = clock();
  t->last = (float)(end - t->start) / CLOCKS_PER_SEC;
  if (t->sum == H3DVF(0))
    t->sum = t->last;
  t->sum = t->avgweight * t->sum + (H3DVF(1) - t->avgweight) * t->last;
  if (t->sum < t->min)
    t->min = t->sum;
  if (t->sum > t->max)
    t->max = t->sum;
}
