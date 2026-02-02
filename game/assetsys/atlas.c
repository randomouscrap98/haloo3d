#include "atlas.h"
#include "store.h"
#include "../../haloo3d.h"
#include "../../haloo3d_ex.h"
#include "../utils/utils.h"
#include "../utils/lists.h"
#include "../utils/log.h"

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

// static inline int atlas_position_compare(atlas_position * a, atlas_position * b) {
//   return a->original_texture->height > b->original_texture->height;
// }


int atlas_create(assetstore * store, uint32_t maxwidth, uint16_t baseline_size,
                 h3d_fb * out, atlas_position ** outpos) {
  NEXTPOW2_32(maxwidth);
  // Prep the tracking for how many PIXELS are filled in any unit-sized row (shelves)
  uint32_t shelves[ATLAS_MAX_HEIGHT_UNITS];
  uint32_t shelvescount = 1; // In units
  memset(shelves, 0, sizeof(shelves));
  // Prealloc the storage of positions
  int32_t texcount = _texturenode_count(store->textures);
  logdebug("Creating an atlas out of %d textures with width %d", texcount, maxwidth);
  *outpos = malloc(sizeof(atlas_position) * texcount);
  if(*outpos == NULL) goto ATLAS_FAIL1;
  // The order of the outpos data doesn't matter. Prefill it so we can sort it
  int32_t idx = 0;
  for(_texturenode * next = store->textures; next; next = next->next) {
    (*outpos)[idx].name = next->name;
    (*outpos)[idx].original_texture = &next->texture;
    idx++;
  }
  LIST_SORT((*outpos), texcount, atlas_position, ATLAS_POSITION_SORT_HEIGHT_DESC);
  // Before we create a texture, let's actually simulate the fill first. This prevents
  // unnecessary mallocs. This IS the work!
  for(idx = 0; idx < texcount; idx++) {
    h3d_fb * tex = (*outpos)[idx].original_texture;
    if(tex->width > maxwidth) {
      logerror("Texture %s doesn't fit within max width: %d > %d!", (*outpos)[idx].name, tex->width, maxwidth);
      goto ATLAS_FAIL2;
    }
    uint32_t unitheight = DIVROUNDUP(tex->height, baseline_size);
    int32_t foundshelf = -1;
    for(uint32_t j = 0; j <= shelvescount - unitheight; j++) {
      if(unitheight > 1) { // Do an extra check to see if the next few rows are aligned
        for(uint32_t rofs = 1; rofs < unitheight; rofs++) {
          // We can check just for equality because we sort by largest first
          if(shelves[rofs + j] != shelves[j]) goto ATLAS_BADROW;
        }
      }
      // This may work for height, but now need to see if it will fit. easy
      if(tex->width + shelves[j] <= maxwidth) {
        foundshelf = j;
        break;
      }
      ATLAS_BADROW:;
    }
    if(foundshelf < 0) { // Need to add a new row or five
      foundshelf = shelvescount;
      shelvescount += unitheight;
      if(shelvescount > ATLAS_MAX_HEIGHT_UNITS) {
        logerror("Textures don't fit into restricted unit height! Failed at texture %d(%s)", idx, (*outpos)[idx].name);
        goto ATLAS_FAIL2;
      }
    }
    // And now, we have a place to put the texture
    VEC2((*outpos)[idx].offset, shelves[foundshelf], foundshelf * baseline_size);
    for(uint32_t j = foundshelf; j < foundshelf + unitheight; j++) {
      shelves[j] += tex->width;
    }
  }
  // Now actually build the texture and put everything together!
  H3D_FB_INIT(out, maxwidth, shelvescount * baseline_size, 2); 
  for(idx = 0; idx < texcount; idx++) {
    // Is this really all I have for copying textures into other textures?
    h3d_fb_intscale((*outpos)[idx].original_texture, out, (*outpos)[idx].offset[0], (*outpos)[idx].offset[1], 1);
  }
  return 0;
ATLAS_FAIL2:
  free(*outpos);
ATLAS_FAIL1:
  return -1;
}
