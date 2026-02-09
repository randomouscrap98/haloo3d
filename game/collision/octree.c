#include "octree.h"

#include <float.h>
#include "collision.h"
#include "../../haloo3d.h"
#include "../../haloo3d_3d.h"
#include "../utils/log.h"
#include "../utils/utils.h"

VECTOR_DEFINE(ocfpointer);
VECTOR_DEFINE(octree_node);

void octree_node_init(octree_node * node) {
  VEC3(node->min, 0, 0, 0);
  VEC3(node->max, 0, 0, 0);
  node->faces_count = 0;
  node->faces_index = UINT32_MAX;
  node->children_index = UINT32_MAX;
}

int octree_node_is_leaf(octree_node * node) {
  return node->children_index == UINT32_MAX;
}

int octree_init(octree * tree) {
  tree->model = NULL;
  tree->facecache = NULL;
  int result = vector_ocfpointer_init(&tree->faces);
  if(result) goto END;
  result = vector_octree_node_init(&tree->nodes);
  if(result) goto ERROR1;
  goto END;
//ERROR2:
  vector_octree_node_free(&tree->nodes);
ERROR1:
  vector_ocfpointer_free(&tree->faces);
END:
  return result;
}

void octree_free(octree * tree) {
  vector_octree_node_free(&tree->nodes);
  vector_ocfpointer_free(&tree->faces);
  tree->model = NULL; // CAREFUL! We don't free your model for you!!
  NULLFREE(tree->facecache);
}

int octree_recurse(octree* tree, size_t parentidx, vector_ocfpointer * parentfaces, uint32_t depth) {
  // First, calculate which tris are in this octree node. 
  vector_ocfpointer nodefaces;
  int result = vector_ocfpointer_init(&nodefaces);
  if(result) {
    logerror("Can't initialize temp tri storage for node at depth %d", depth);
    goto END;
  }
  //printf("parentidx %zu parentfaces length %zu nodes %zu\n", parentidx, parentfaces->length, tree->nodes.length);
  //printf("parentface0 pointer: %p\n", parentfaces->array[0]);
  if(depth == 0) {
    // A pure copy
    result = vector_ocfpointer_append(&nodefaces, parentfaces);
    if(result) {
      logerror("Can't append tri storage for node at depth %d", depth);
      goto ERROR1;
    }
  } else {
    // Go through each collision box in faces and see which collide with us
    for(size_t i = 0; i < parentfaces->length; i++) {
      if(AABBCOLLIDE_3DBOX(parentfaces->array[i]->min, parentfaces->array[i]->max, 
                           tree->nodes.array[parentidx].min, tree->nodes.array[parentidx].max)) {
        result = vector_ocfpointer_push(&nodefaces, &parentfaces->array[i]);
        if(result) {
          logerror("Can't append face for node faces at depth %d", depth);
          goto ERROR1;
        }
      }
    }
  }
  // We now have a list of faces which are inside this node. Let's use a heuristic
  // to determine if we need to split
  //if(nodefaces.length > 2 * (depth + 1)) {
  if(nodefaces.length > 2 * depth) {
    // Split time. Figure out the bounds of the 8 octants and then recurse into
    // them using our triangle nodes as the faces
    vec3 dim;
    vec3 offset;
    vec3_subtract(tree->nodes.array[parentidx].max, tree->nodes.array[parentidx].min, dim);
    vec3_multiply(dim, 0.5, dim);
    for(int z = 0; z < 2; z++) {
      for(int y = 0; y < 2; y++) {
        for(int x = 0; x < 2; x++) {
          size_t idx;
          result = vector_octree_node_increment(&tree->nodes, &idx);
          if(result) {
            logerror("Can't append node at depth %d", depth);
            goto ERROR1;
          }
          octree_node * parentnode = &tree->nodes.array[parentidx]; // ONLY here can we use this...
          octree_node * newnode = &tree->nodes.array[idx];
          octree_node_init(newnode);
          VEC3(offset, x * dim[0], y * dim[1], z * dim[2]);
          vec3_add(parentnode->min, offset, newnode->min);
          vec3_add(newnode->min, dim, newnode->max);
          result = octree_recurse(tree, idx, &nodefaces, depth + 1);
          // NOTE: no node pointer usable after this call!!
          if(result) {
            logerror("Can't recurse node at depth %d", depth);
            goto ERROR1;
          }
        }
      }
    }
  } else {
    // It's a leaf. Let's append the faces to the global face cache
    tree->nodes.array[parentidx].faces_count = nodefaces.length;
    tree->nodes.array[parentidx].faces_index = tree->faces.length;
    result = vector_ocfpointer_append(&tree->faces, &nodefaces);
    if(result) {
      logerror("Can't append tris for node at depth %d", depth);
      goto ERROR1;
    }
  }
ERROR1:
  vector_ocfpointer_free(&nodefaces);
END:
  return 0;
}

// Octree build requires such a huge initialization, just make it easier on myself.
// Nothing that is initialized here needs to be freed on error
static int octree_build_init(octree * tree, uint32_t numfaces, size_t * rootidx) {
  // Clear out all the old crap
  vector_octree_node_clear(&tree->nodes);
  vector_ocfpointer_clear(&tree->faces);
  NULLFREE(tree->facecache);

  // Allocate the face cache
  tree->facecache = malloc(sizeof(octree_face) * numfaces);
  if(!tree->facecache) {
    logerror("Couldn't initialize face cache");
    return 1;
  }
  // Note: we DON'T free the facecache on error!

  // Make the root node, it's always the entire thing.
  if(vector_octree_node_increment(&tree->nodes, rootidx)) {
    logerror("Couldn't initialize root octree node");
    return 1;
  }
  // Root node won't move here, but may move later. be careful with pointer...
  octree_node * root = &tree->nodes.array[*rootidx];
  octree_node_init(root);
  VEC3(root->min, FLT_MAX, FLT_MAX, FLT_MAX);
  VEC3(root->max, FLT_MIN, FLT_MIN, FLT_MIN);

  return 0;
}

static int octree_build_rootfaces(octree * tree, vector_ocfpointer * roottris, uint32_t numfaces) {
  // For speed, pre-reserve the right amount (so we have an exact size and
  // aren't reallocating over and over)
  if(vector_ocfpointer_reserve(roottris, numfaces)) {
    logerror("Couldn't reserve space in root triangle vector");
    return 1;
  }
  // Root triangle list is literally just a copy of the tri cache
  for(uint32_t i = 0; i < numfaces; i++) {
    ocfpointer fp = tree->facecache + i;
    if(vector_ocfpointer_push(roottris, &fp)) {
      logerror("Couldn't add root face");
      return 1;
    }
  }
  return 0;
}

int octree_build(octree * tree, h3d_obj * model) {
  tree->model = model;

  size_t rootidx;
  int result = octree_build_init(tree, model->numfaces, &rootidx);
  if(result) { goto END; }

  // Allocate the root triangle list
  vector_ocfpointer roottris;
  result = vector_ocfpointer_init(&roottris);
  if(result) {
    logerror("Couldn't initialize root triangle vector");
    goto END;
  }

  // Rootfaces is simple mapping directly to the cache
  result = octree_build_rootfaces(tree, &roottris, model->numfaces);
  if(result) { goto ERROR1; }

  octree_node * root = &tree->nodes.array[rootidx];

  // build a vector of all faces for the root node. In the meantime, also keep
  // track of the max and min positions of all vertices 
  for(uint32_t i = 0; i < model->numfaces; i++) {
    octree_face * tri = tree->facecache + i;
    collision_objface_aabb(model, i, tri->min, tri->max);
    // We have aabb now, just do min/max for this vs our global min/max
    for(int pi = 0; pi < 3; pi++) { // for each dimension
      root->min[pi] = H3D_MIN(root->min[pi], tri->min[pi]);
      root->max[pi] = H3D_MAX(root->max[pi], tri->max[pi]);
    }
  }
  
  // We now have list of all tris and the global dimensions of the octree.
  // We can now start recursing. Once the recursion is done, we should have
  // a fully valid tree with everything allocated
  result = octree_recurse(tree, rootidx, &roottris, 0);

ERROR1:
  // We don't need the roottris
  vector_ocfpointer_free(&roottris);
END:
  return result;

  // First, we know all 
  // Steps:
  // - see if triangle count in region (how?) is too high
  // - if so, generate 8 octree nodes and put triangles into them
  // - repeat recursively

  // Triangles may be in multiple regions, can't just move everything over...
  // - may need to keep temporary vectors just for this as you pass stuff around
  // - lots of malloc and free to build the tree....

}
