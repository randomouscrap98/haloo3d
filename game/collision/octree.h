#ifndef __COLLISION_OCTREE_H__
#define __COLLISION_OCTREE_H__

#include <stdint.h>
#include "../utils/vector.h"
#include "../../haloo3d_obj.h"

// A single cube of space which may either point to 8 more cubes
// or simply point to triangles inside itself
typedef struct {
  vec3 min;
  vec3 max;
  uint32_t children_index; // if special value INT_MAX, no children (leaf node)
  // NOTE: children count always 8 (no need to store it)
  uint32_t faces_index;
  uint16_t faces_count;
} octree_node;

void octree_node_init(octree_node * node);
int octree_node_is_leaf(octree_node * node);

// We store some cached data for the triangle to prevent tons of if statements
// from running like 10,000 times (probaby not a huge deal though)
typedef struct {
  vec3 min;     // Position of calculated AABB box
  vec3 max;     // Position of calculated AABB box
} octree_face;

// Note: to limit the need to pull weird vector implementations
// in, we're going to just typedef the stuff we need and create
// unique vector types. We'll see how this works 2026-02-07
typedef octree_face * ocfindex;

VECTOR_DECLARE(ocfindex);
VECTOR_DECLARE(octree_node);

// Real octree is a container of linear octree nodes which point at each
// other, rather than a chain of references. Simpler initialization
// and free this way. NOTE: this octree expects FULLY STATIC geometry!!!
typedef struct {
  h3d_obj * model;          // Reference to object this octree is pointing at
  vector_octree_node nodes; // Global storage for all nodes
  vector_ocfindex faces;    // Global storage for all pointers into face cache
  octree_face * facecache;  // Look into this for cached
} octree;

int octree_init(octree * tree);
void octree_free(octree * tree);
int octree_build(octree * tree, h3d_obj * model);

// Fill the given out vector with all the faces which MAY need to be check for
// collision with the given 3d box at pos with dim. Note that the pos needs to
// be in the same space as the model...
// int octree_get_pending_collisions(octree * tree, vec3 pos, vec3 dim, 
//     vector_ocfindex * out);

#endif
