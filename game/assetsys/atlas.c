#include "atlas.h"
#include "store.h"

uint16_t atlas_baseline_texture_unit(assetstore * store) {
  // Storage for baseline dimension tracking
  uint16_t dimnext = 0;
  uint16_t dimension[ATLAS_UNIT_MAXCHECK];
  uint16_t dimcounts[ATLAS_UNIT_MAXCHECK];
  // Iterate over all textures
  for(_texturenode * next = store->textures; next; next = next->next) {
    int found = 0;
    for(uint16_t i = 0; i < dimnext; i++) {
      if(dimension[i] == next->texture.height) {
        dimcounts[i]++;
        found = 1;
        break;
      }
    }
    if(!found && dimnext < ATLAS_UNIT_MAXCHECK) {
      dimension[dimnext] = next->texture.height;
      dimcounts[dimnext] = 1;
      dimnext++;
    }
  }
  if(dimnext == 0) {
    return 0; // No dimensions? No unit
  }
  uint16_t maxcount = 0;
  uint16_t maxdim = 0;
  for(uint16_t i = 0; i < dimnext; i++) {
    if(dimcounts[i] > maxcount) {
      maxcount = dimcounts[i];
      maxdim = dimension[i];
    }
  }
  return maxdim;
}

int atlas_create(assetstore * store, uint32_t maxwidth, h3d_fb * out, atlas_position ** outpos) {
  // First, use a heuristic to figure out what the rough "approximate" texture size
  // is. We count the amounts of times a dimension shows up and just use that. We
  // only track the top 1024 sizes
  return 0;
}
