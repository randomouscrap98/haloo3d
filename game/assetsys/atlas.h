#ifndef __ASSETSYS_ATLAS_H__
#define __ASSETSYS_ATLAS_H__

#include "store.h"

#define ATLAS_UNIT_MAXCHECK 1024

typedef struct {
  char * name;                  // Points back INTO the assetstore, careful!
  h3d_fb * original_texture;    // Points back INTO the assetstore, careful!
  vec2i offset;
} atlas_position;

// Calculate a rough "baseline" texture unit size for all the textures in the store.
// For instance, if you're making a block game with 16x16 textures, your computed
// baseline texture unit may be 16. Only considers the height (because of the algorithm
// used in atlas_create)
uint16_t atlas_baseline_texture_unit(assetstore * store);

// Using all of the textures in the given assetstore, put them all together
// into a single texture in the given out, with the given rough "maximum width".
// Max width will be rounded up to the next power of two (to allow textures to function)
// Pass in an unassigned pointer as the last arg to get a list of all the textures and 
// where they are in the atlas. Pass in an unitialized texture for out, this algorithm 
// will fill it. This uses a variation of the "shelf" algorithm and thus does not produce 
// particularly good results, but if you're using mostly standard sized textures and not 
// random sizes, this should suffice.
int atlas_create(assetstore * store, uint32_t maxwidth, h3d_fb * out, atlas_position ** outpos);

#endif
