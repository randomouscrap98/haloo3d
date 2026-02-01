#include "atlas.h"
#include "../../haloo3d.h"
#include "../../haloo3d_ex.h"
#include "../utils/utils.h"
#include "../utils/test.h"
#include "../utils/log.h"
#include "store.h"

static void _fill_fb(h3d_fb * fb, int buff) {
  for(int i = 0; i < H3D_FB_SIZE(fb); i++) {
    fb->buffer[i] = i + buff;
  }
}

static void _no_overlap(atlas_position * pos, int length) {
  for(int i = 0; i < length - 1; i++) {
    for(int j = i + 1; j < length; j++) {
      vec2 dimi, dimj;
      VEC2(dimi, pos[i].original_texture->width, pos[i].original_texture->height);
      VEC2(dimj, pos[j].original_texture->width, pos[j].original_texture->height);
      if(AABBCOLLIDE(pos[i].offset, dimi, pos[j].offset, dimj)) {
        logfatal("Textures %d(%s) and %d(%s) overlap", i, pos[i].name, j, pos[j].name);
      }
    }
  }
}

static void _compare_fb(h3d_fb * atlas, atlas_position * pos) {
  for(int y = 0; y < pos->original_texture->height; y++) {
    for(int x = 0; x < pos->original_texture->width; x++) {
      //int i = x + y * pos->original_texture->width;
      int atlasval = H3D_FB_GET(atlas, pos->offset[0] + x, pos->offset[1] + y);
      int fbval = H3D_FB_GET(pos->original_texture, x, y);
      if(atlasval != fbval) {
        logfatal("Atlas doesn't match fb(%s) at offset (%d,%d)", pos->name, x, y);
      }
    }
  }
}

void atlas_test() {
  assetstore store;
  assetstore_init(&store);
  h3d_fb * basic = assetstore_new_texture(&store, "one");
  H3D_FB_INIT(basic, 16, 16, 2);
  _fill_fb(basic, 0);

  uint16_t basesize = atlas_baseline_texture_unit(&store);
  ASSERT(basesize == 16, "atlas_baseline_texture_unit(1)");

  h3d_fb atlas;
  atlas_position * positions;
  int result = atlas_create(&store, 16, basesize, &atlas, &positions);
  ASSERT(result == 0, "atlas_create result 0");

  _compare_fb(&atlas, &positions[0]);
  _no_overlap(positions, _texturenode_count(store.textures));
  //int32_t texcount = _texturenode_count(store.textures);

  H3D_FB_FREE(&atlas);
}
