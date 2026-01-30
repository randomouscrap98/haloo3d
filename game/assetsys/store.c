#include "store.h"
#include <string.h>

#include "../../haloo3d_ex.h"
#include "../utils/log.h"
#include "../utils/mystring.h"

LL_DEFINITION(_texturenode);
LL_DEFINITION(_objectnode);

void assetstore_init(assetstore * store) {
  _texturenode_init(&store->textures);
  _objectnode_init(&store->objects);
  store->texture_count = 0;
  store->object_count = 0;
}

static _texturenode * assetstore_find_texturenode(assetstore * store, const char * name) {
  for(_texturenode * n = store->textures; n; n = n->next) {
    if(strcmp(name, n->name) == 0) {
      return n;
    }
  }
  return NULL;
}

static _objectnode * assetstore_find_objectnode(assetstore * store, const char * name) {
  for(_objectnode * n = store->objects; n; n = n->next) {
    if(strcmp(name, n->name) == 0) {
      return n;
    }
  }
  return NULL;
}

h3d_fb * assetstore_new_texture(assetstore * store, const char * name) {
  // This is an IMMEDIATE DEATH
  if(assetstore_find_texturenode(store, name)) {
    logfatal("Texture with same key inserted: %s", name);
  }
  _texturenode * out;
  int result = _texturenode_create_before(&store->textures, NULL, &out);
  if(result) return NULL;
  strncpy(out->name, name, ASSETMAXNAME);
  out->name[ASSETMAXNAME] = 0;
  store->texture_count++;
  return &out->texture;
}

h3d_obj * assetstore_new_object(assetstore * store, const char * name) {
  // This is an IMMEDIATE DEATH
  if(assetstore_find_objectnode(store, name)) {
    logfatal("Object with same key inserted: %s", name);
  }
  _objectnode * out;
  int result = _objectnode_create_before(&store->objects, NULL, &out);
  if(result) return NULL;
  strncpy(out->name, name, ASSETMAXNAME);
  out->name[ASSETMAXNAME] = 0;
  store->object_count++;
  return &out->object;
}

h3d_fb * assetstore_find_texture(assetstore * store, const char * name) {
  _texturenode * result = assetstore_find_texturenode(store, name);
  return result ? &result->texture : NULL;
}

h3d_obj * assetstore_find_object(assetstore * store, const char * name) {
  _objectnode * result = assetstore_find_objectnode(store, name);
  return result ? &result->object : NULL;
}

h3d_fb * assetstore_must_find_texture(assetstore * store, const char * name) {
  h3d_fb * result = assetstore_find_texture(store, name);
  if(!result) {
    logfatal("Couldn't find texture %s", name);
  }
  return result;
}

h3d_obj * assetstore_must_find_object(assetstore * store, const char * name) {
  h3d_obj * result = assetstore_find_object(store, name);
  if(!result) {
    logfatal("Couldn't find object %s", name);
  }
  return result;
}

int assetstore_remove_texture(assetstore * store, const char * name) {
  _texturenode * result = assetstore_find_texturenode(store, name);
  if(result == NULL) return 1;
  _texturenode_remove(&store->textures, result);
  store->texture_count--;
  return 0;
}

int assetstore_remove_object(assetstore * store, const char * name) {
  _objectnode * result = assetstore_find_objectnode(store, name);
  if(result == NULL) return 1;
  _objectnode_remove(&store->objects, result);
  store->object_count--;
  return 0;
}

static void tnode_free_internal(_texturenode * node) {
  H3D_FB_FREE(&node->texture);
}

static void onode_free_internal(_objectnode * node) {
  h3d_obj_free(&node->object);
}

void assetstore_free(assetstore * store) {
  _texturenode_free(&store->textures, tnode_free_internal);
  _objectnode_free(&store->objects, onode_free_internal);
}

int assetstore_load_all_textures(assetstore * store, const char * folder, const char * manifestname) {
  int count = 0;
  char path[ASSETMAXPATH];
  char name[ASSETMAXNAME];
  snprintf(path, ASSETMAXPATH, "%s/%s", folder, manifestname);
  FILE * manifest = fopen(path, "r");
  if (!manifest) {
    logfatal("Can't open manifest file %s", path);
  }
  while(fgets(name, ASSETMAXNAME, manifest)) {
    string_trim(name);
    if(name[0] == '\0') continue;
    snprintf(path, ASSETMAXPATH, "%s/%s.ppm", folder, name);
    h3d_fb * fb = assetstore_new_texture(store, name);
    h3d_fb_loadppmfile(fb, path, h3d_fb_in_A4R4G4B4);
    logdebug("Loaded texture %s", path);
    count++;
  }
  fclose(manifest);
  return count;
}

int assetstore_load_all_objects(assetstore * store, const char * folder, const char * manifestname) {
  int count = 0;
  char path[ASSETMAXPATH];
  char name[ASSETMAXNAME];
  snprintf(path, ASSETMAXPATH, "%s/%s", folder, manifestname);
  FILE * manifest = fopen(path, "r");
  if (!manifest) {
    logfatal("Can't open manifest file %s", path);
  }
  while(fgets(name, ASSETMAXNAME, manifest)) {
    string_trim(name);
    if(name[0] == '\0') continue;
    snprintf(path, ASSETMAXPATH, "%s/%s.obj", folder, name);
    h3d_obj * obj = assetstore_new_object(store, name);
    h3d_obj_loadfile(obj, path, ASSETMAXFACES, ASSETMAXVERTICES);
    logdebug("Loaded object %s", path);
    count++;
  }
  fclose(manifest);
  return count;
}
