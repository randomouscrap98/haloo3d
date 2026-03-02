#ifndef __COLLISION_OCTREE_H__
#define __COLLISION_OCTREE_H__

#include <stdint.h>
#include "collision.h"
#include "../utils/vector.h"
#include "../../haloo3d_obj.h"

// A single cube of space which may either point to 8 more cubes
// or simply point to triangles inside itself
typedef struct {
  collision_box_3d box;
  uint32_t children_index; // if special value INT_MAX, no children (leaf node)
  // NOTE: children count always 8 (no need to store it)
  uint32_t faces_index;   // Index into faces vector, which itself is an array of face indexes into obj
  uint16_t faces_count;
} octree_node;

VECTOR_DECLARE(octree_node);

void octree_node_init(octree_node * node);
int octree_node_is_leaf(octree_node * node);

// Note: to limit the need to pull weird vector implementations
// in, we're going to just typedef the stuff we need and create
// unique vector types. We'll see how this works 2026-02-07
typedef uint32_t ocfoffset;

VECTOR_DECLARE(ocfoffset);

// Real octree is a container of linear octree nodes which point at each
// other, rather than a chain of references. Simpler initialization
// and free this way. NOTE: this octree expects FULLY STATIC geometry!!!
typedef struct {
  h3d_obj * model;          // Reference to object this octree is pointing at
  vector_octree_node nodes; // Global storage for all nodes
  vector_ocfoffset nodefaces;   // Global storage for all pointers into face cache
  collision_box_3d * faceboxes;  // Look into this for all the boxes of tris (one to one mapping with obj)
} octree;

int octree_init(octree * tree);
void octree_free(octree * tree);
int octree_build(octree * tree, h3d_obj * model);

// typedef struct {
//   int current_node;
// } octree_scan_data;

typedef int (*octree_scan_callback)(void * state, h3d_obj * obj, uint32_t face);
int octree_scan(octree * tree, collision_box_3d * box, octree_scan_callback callback, void * state);

// Fill the given out vector with all the faces which MAY need to be check for
// collision with the given 3d box at pos with dim. Note that the pos needs to
// be in the same space as the model...
// int octree_get_pending_collisions(octree * tree, vec3 pos, vec3 dim, 
//     vector_ocfindex * out);

#endif
