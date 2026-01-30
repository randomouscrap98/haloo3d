#ifndef __ASSETSYS_STORE_H__
#define __ASSETSYS_STORE_H__

#include "../../haloo3d.h"
#include "../../haloo3d_obj.h"
#include "../utils/linkedlist.h"

#define ASSETMAXNAME 32
#define ASSETMAXPATH 8192
// These are just for loading from a manifest. If you're loading 
// your own objects, you can use whatever you want
#define ASSETMAXFACES 65536
#define ASSETMAXVERTICES 65536

// Internal linked list node. You don't want to manipulate this yourself!
typedef struct _texturenode {
  h3d_fb texture;
  char name[ASSETMAXNAME + 1];
  LL_FIELDS(_texturenode);
} _texturenode;

LL_PROTOTYPE(_texturenode);

// Internal linked list node. You don't want to manipulate this yourself!
typedef struct _objectnode {
  h3d_obj object;
  char name[ASSETMAXNAME + 1];
  LL_FIELDS(_objectnode);
} _objectnode;

LL_PROTOTYPE(_objectnode);

// A container to store an unlimited number of
// objects and textures by name. Not very efficient; linear scan
typedef struct {
  _texturenode * textures;
  _objectnode * objects;
  int texture_count;
  int object_count;
} assetstore;

void assetstore_init(assetstore * store);
void assetstore_free(assetstore * store);
// Allocate space for a new texture (UNITIALIZED) and return it
h3d_fb * assetstore_new_texture(assetstore * store, const char * name);
// Allocate space for a new object (UNITIALIZED) and return it
h3d_obj * assetstore_new_object(assetstore * store, const char * name);
// Find texture by name. Returns null if not found
h3d_fb * assetstore_find_texture(assetstore * store, const char * name);
// Find object by name. Returns null if not found
h3d_obj * assetstore_find_object(assetstore * store, const char * name);
// Find texture by name. Fails if texture not found
h3d_fb * assetstore_must_find_texture(assetstore * store, const char * name);
// Find object by name. Returns null if not found
h3d_obj * assetstore_must_find_object(assetstore * store, const char * name);
// Remove texture with given name. If not found, nonzero status returned.
// Return value is NOT index
int assetstore_remove_texture(assetstore * store, const char * name);
// Remove texture with given name. If not found, nonzero status returned.
// Return value is NOT index
int assetstore_remove_object(assetstore * store, const char * name);

// Load all ppm assets from the given folder. Expects a manifest file
int assetstore_load_all_textures(assetstore * store, const char * folder, 
                                 const char * manifestname);
// Load all obj assets from the given folder. Expects a manifest file
int assetstore_load_all_objects(assetstore * store, const char * folder, 
                                const char * manifestname);

#endif
