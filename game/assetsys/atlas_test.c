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
  logdebug("Check no_overlap for %d positions", length);
  for(int i = 0; i < length - 1; i++) {
    vec2 dimi, dimj;
    VEC2(dimi, pos[i].original_texture->width, pos[i].original_texture->height);
    for(int j = i + 1; j < length; j++) {
      VEC2(dimj, pos[j].original_texture->width, pos[j].original_texture->height);
      if(AABBCOLLIDE(pos[i].offset, dimi, pos[j].offset, dimj)) {
        logfatal("Textures %d(%s) and %d(%s) overlap", i, pos[i].name, j, pos[j].name);
      }
    }
  }
}

static void _compare_fb(h3d_fb * atlas, atlas_position * pos) {
  logdebug("Compare atlas for %s at %d,%d", pos->name, pos->offset[0], pos->offset[1]);
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

static void gen_and_check_atlas(assetstore * store, h3d_fb * atlas, atlas_position ** positions, 
                                uint16_t width, uint16_t exp_height) {
  int texcount = _texturenode_count(store->textures);
  H3D_FB_FREE(atlas); free(*positions);
  uint16_t basesize = atlas_baseline_texture_unit(store);
  int result = atlas_create(store, width, basesize, atlas, positions);
  ASSERT(result == 0, "atlas_create%d result 0", texcount);
  ASSERT(atlas->width == width, "atlas_create%d width %d", texcount, width);
  ASSERT(atlas->height == exp_height, "atlas_create%d height %d", texcount, exp_height);

  _no_overlap(*positions, texcount);
  for(int i = 0; i < texcount; i++) {
    _compare_fb(atlas, &(*positions)[i]);
  }
}

void atlas_test() {
  assetstore store;
  assetstore_init(&store);
  //assetstore atlases;
  //assetstore_init(&atlases);
  //h3d_fb * textures[1024]; 
  //int texcount = 0;
  //textures[texcount++] = assetstore_new_texture(&store, "one");
  h3d_fb * basic = assetstore_new_texture(&store, "one");
  H3D_FB_INIT(basic, 16, 16, 2);
  _fill_fb(basic, 0);

  uint16_t basesize = atlas_baseline_texture_unit(&store);
  ASSERT(basesize == 16, "atlas_baseline_texture_unit(1)");

  h3d_fb atlas;// = assetstore_new_texture(&atlases, "single");
  atlas_position * positions;
  int result = atlas_create(&store, 16, basesize, &atlas, &positions);
  ASSERT(result == 0, "atlas_create result 0");
  ASSERT(atlas.width == 16, "atlas_create width 16");
  ASSERT(atlas.height == 16, "atlas_create height 16");

  _compare_fb(&atlas, &positions[0]);
  _no_overlap(positions, _texturenode_count(store.textures));

  // -- Now try with two textures --
  basic = assetstore_new_texture(&store, "two");
  H3D_FB_INIT(basic, 16, 16, 2);
  _fill_fb(basic, 256);

  gen_and_check_atlas(&store, &atlas, &positions, 16, 32);
  gen_and_check_atlas(&store, &atlas, &positions, 32, 16);

  // -- Now try with three textures (all still the same size) --
  basic = assetstore_new_texture(&store, "three");
  H3D_FB_INIT(basic, 16, 16, 2);
  _fill_fb(basic, 2*256);

  gen_and_check_atlas(&store, &atlas, &positions, 16, 48);
  gen_and_check_atlas(&store, &atlas, &positions, 32, 32);
  gen_and_check_atlas(&store, &atlas, &positions, 64, 16);

  //assetstore_free(&atlases);
  H3D_FB_FREE(&atlas); free(positions);
  assetstore_free(&store);
}
