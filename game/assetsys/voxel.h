#ifndef __ASSETSYS_VOXEL_H__
#define __ASSETSYS_VOXEL_H__

#include "store.h"

#define ATLAS_ACROSS 64

// Vtexture indexes into the obj 
typedef struct {
  uint16_t front;
  uint16_t back;
  uint16_t left;
  uint16_t right;
  uint16_t top;
  uint16_t bottom;
} atlas_vtex_mapping;

// Create a single atlas texture for specialized voxel textures in the 
// given texture storage. Creates the texture for you; pass in an 
// unitialized texture. Stores every possible texture vertex coordinate 
// within outobj, also pass in an unitialized one.
// Uses all the textures in the given assetstore that parse to numbers. 
// Numbers are ids which index into the 
// Expects all textures to be a consistent size. Different dimensions mean different things:
// 1:1 means all six sides are the same. 1:2 means sides, then top.
// 1:3 means sides, top, and bottom. 1:4 means front/back/top/bottom.
// 1:6 means front,back,left,right,top,bottom.
// To use:
// - Load all desired textures with integer names into assetstore
// - Prep a blank object and texture (can put it in another assetstore?)
// - Prep an unasigned pointer to atlas vtex mapping
// - Call the function
// - You can probably delete the old assetstore, since all the textures were 
//   copied into the atlas
// - outtex can be attached to any model which holds these voxels
int create_voxel_atlas(assetstore * store, 
    h3d_fb * outtex, h3d_obj * outobj, atlas_vtex_mapping ** outmap);

// TODO:
// The above is probably too complicated and also too rigid. Consider:
// - an atlas that can hold ANY combination of textures
// - return information about where it is
// - decide for yourself later how everything maps into it etc.
// - Don't limit to just integer names, just take the whole assetstore.
//   mapping is offset and width/height, and index into mapping is index
//   into assetstore (make sure you can iterate assetstore linearly)
// - Move into generic atlas in assetsys since it's not related to voxels
// - Have a different function scan through the mapping and figure out how to...
//   do voxel top/bottom/etc and stuff? IDK how much THAT function will do
// - Consider how non-solid blocks will work inside the atlas. Perhaps mapping
//   only based on whether id is for a solid block, then others... you just use
//   the region? idk...

#endif
