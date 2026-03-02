#include "octree.h"

#include <float.h>
#include "../../haloo3d.h"
#include "../../haloo3d_3d.h"
#include "../utils/log.h"
#include "../utils/utils.h"
#include "../utils/print.h"
#include "collision.h"

VECTOR_DEFINE(ocfoffset);
VECTOR_DEFINE(octree_node);

// Uncomment for mega logging
#define OCTREE_VERBOSE_LOG

void octree_node_init(octree_node * node) {
  collision_box_3d_init(&node->box);
  node->faces_count = 0;
  node->faces_index = UINT32_MAX;
  node->children_index = UINT32_MAX;
}

int octree_node_is_leaf(octree_node * node) {
  return node->children_index == UINT32_MAX;
}

int octree_init(octree * tree) {
  tree->model = NULL;
  tree->faceboxes = NULL;
  int result = vector_ocfoffset_init(&tree->nodefaces);
  if(result) goto END;
  result = vector_octree_node_init(&tree->nodes);
  if(result) goto ERROR1;
  goto END;
//ERROR2:
  vector_octree_node_free(&tree->nodes);
ERROR1:
  vector_ocfoffset_free(&tree->nodefaces);
END:
  return result;
}

void octree_free(octree * tree) {
  vector_octree_node_free(&tree->nodes);
  vector_ocfoffset_free(&tree->nodefaces);
  tree->model = NULL; // CAREFUL! We don't free your model for you!!
  NULLFREE(tree->faceboxes);
}

#define __OCTRU_P() logdebug( \
  "OCTREE_RECURSE: depth %d quadrant %d [%d%d%d] " VEC3FMT(3) " -> " VEC3FMT(3), \
  depth, quadrant, quadrant & 1, (quadrant & 2) >> 1, (quadrant & 4) >> 2, \
  VEC3SPREAD(tree->nodes.array[parentidx].box.min), \
  VEC3SPREAD(tree->nodes.array[parentidx].box.max) \
);

int octree_recurse(octree* tree, size_t parentidx, vector_ocfoffset * parentfaces, 
                   uint32_t depth, uint32_t quadrant) {
  // First, calculate which tris are in this octree node. 
  vector_ocfoffset nodefaces;
  int result = vector_ocfoffset_init(&nodefaces);
  if(result) {
    logerror("Can't initialize temp tri storage for node:");
    __OCTRU_P();
    goto END;
  }
  //printf("parentidx %zu parentfaces length %zu nodes %zu\n", parentidx, parentfaces->length, tree->nodes.length);
  //printf("parentface0 pointer: %p\n", parentfaces->array[0]);
  if(depth == 0) {
    // A pure copy
    result = vector_ocfoffset_append(&nodefaces, parentfaces);
    if(result) {
      logerror("Can't append tri storage for node:");
      __OCTRU_P();
      goto ERROR1;
    }
  } else {
    // Go through each collision box in faces and see which collide with us
    octree_node * pnode = tree->nodes.array + parentidx;
    for(size_t i = 0; i < parentfaces->length; i++) {
      collision_box_3d * pbox = tree->faceboxes + parentfaces->array[i];
      if(AABBCOLLIDE_3DBOX(pbox, &pnode->box)) {
        result = vector_ocfoffset_push(&nodefaces, &parentfaces->array[i]);
        if(result) {
          logerror("Can't append face for node faces:");
          __OCTRU_P();
          goto ERROR1;
        }
      }
    }
  }
#ifdef OCTREE_VERBOSE_LOG
  __OCTRU_P();
  logdebug("Nodes here: %zu", nodefaces.length);
#endif
  // We now have a list of faces which are inside this node. Let's use a heuristic
  // to determine if we need to split
  if(nodefaces.length > 2 * (depth + 1)) {
    // Split time. Figure out the bounds of the 8 octants and then recurse into
    // them using our triangle nodes as the faces
    vec3 dim;
    vec3 offset;
    vec3_subtract(tree->nodes.array[parentidx].box.max, tree->nodes.array[parentidx].box.min, dim);
    vec3_multiply(dim, 0.5, dim);
    // Explicit cast and ugh whatever...
    tree->nodes.array[parentidx].children_index = (uint32_t)tree->nodes.length;
    for(int z = 0; z < 2; z++) {
      for(int y = 0; y < 2; y++) {
        for(int x = 0; x < 2; x++) {
          size_t idx;
          result = vector_octree_node_increment(&tree->nodes, &idx);
          if(result) {
            logerror("Can't append node:");
            __OCTRU_P();
            goto ERROR1;
          }
          octree_node * parentnode = &tree->nodes.array[parentidx]; // ONLY here can we use this...
          octree_node * newnode = &tree->nodes.array[idx];
          octree_node_init(newnode);
          VEC3(offset, x * dim[0], y * dim[1], z * dim[2]);
          vec3_add(parentnode->box.min, offset, newnode->box.min);
          vec3_add(newnode->box.min, dim, newnode->box.max);
          result = octree_recurse(tree, idx, &nodefaces, depth + 1, x + (y << 1) + (z << 2));
          // NOTE: no node pointer usable after this call!!
          if(result) {
            logerror("Can't recurse node:");
            __OCTRU_P();
            goto ERROR1;
          }
        }
      }
    }
  } else {
    // It's a leaf. Let's append the faces to the global face cache
    tree->nodes.array[parentidx].faces_count = nodefaces.length;
    tree->nodes.array[parentidx].faces_index = tree->nodefaces.length;
    result = vector_ocfoffset_append(&tree->nodefaces, &nodefaces);
    if(result) {
      logerror("Can't append tris for node:");
      __OCTRU_P();
      goto ERROR1;
    }
  }
ERROR1:
  vector_ocfoffset_free(&nodefaces);
END:
  return result;
}

// Octree build requires such a huge initialization, just make it easier on myself.
// Nothing that is initialized here needs to be freed on error
static int octree_build_init(octree * tree, uint32_t numfaces, size_t * rootidx) {
  // Clear out all the old crap
  vector_octree_node_clear(&tree->nodes);
  vector_ocfoffset_clear(&tree->nodefaces);
  NULLFREE(tree->faceboxes);

  // Allocate the face cache
  tree->faceboxes = malloc(sizeof(collision_box_3d) * numfaces);
  if(!tree->faceboxes) {
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
  VEC3(root->box.min, FLT_MAX, FLT_MAX, FLT_MAX);
  VEC3(root->box.max, -FLT_MAX, -FLT_MAX, -FLT_MAX);

  return 0;
}

static int octree_build_rootfaces(vector_ocfoffset * roottris, uint32_t numfaces) {
  // For speed, pre-reserve the right amount (so we have an exact size and
  // aren't reallocating over and over)
  if(vector_ocfoffset_reserve(roottris, numfaces)) {
    logerror("Couldn't reserve space in root triangle vector");
    return 1;
  }
  // Root triangle list is literally just a copy of the tri cache
  for(uint32_t i = 0; i < numfaces; i++) {
    //ocfpointer fp = tree->facecache + i;
    if(vector_ocfoffset_push(roottris, &i)) {
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
  vector_ocfoffset rootboxes;
  result = vector_ocfoffset_init(&rootboxes);
  if(result) {
    logerror("Couldn't initialize root triangle vector");
    goto END;
  }

  // Rootfaces is simple mapping directly to the cache
  result = octree_build_rootfaces(&rootboxes, model->numfaces);
  if(result) { goto ERROR1; }

  octree_node * root = &tree->nodes.array[rootidx];

  // build a vector of all faces for the root node. In the meantime, also keep
  // track of the max and min positions of all vertices 
  for(uint32_t i = 0; i < model->numfaces; i++) {
    collision_box_3d * box = tree->faceboxes + i;
    collision_objface_aabb(model, i, box);
#ifdef OCTREE_VERBOSE_LOG
    logdebug("Face %d: "VEC3FMT(3)" -> "VEC3FMT(3), i, 
             VEC3SPREAD(box->min), VEC3SPREAD(box->max));
#endif
    // We have aabb now, just do min/max for this vs our global min/max
    for(int pi = 0; pi < 3; pi++) { // for each dimension
      root->box.min[pi] = H3D_MIN(root->box.min[pi], box->min[pi]);
      root->box.max[pi] = H3D_MAX(root->box.max[pi], box->max[pi]);
    }
  }
  
  // We now have list of all tris and the global dimensions of the octree.
  // We can now start recursing. Once the recursion is done, we should have
  // a fully valid tree with everything allocated
  result = octree_recurse(tree, rootidx, &rootboxes, 0, 8);

ERROR1:
  // We don't need the roottris
  vector_ocfoffset_free(&rootboxes);
END:
  return result;
}


static int octree_scan_recurse(octree * tree, collision_box_3d * box, octree_node * node, 
                               octree_scan_callback callback, void * state) {
  if(AABBCOLLIDE_3DBOX(box, &node->box)) {
    //logdebug("COLLIDE NODE: %d", (int)(node - tree->nodes.array));
    if(octree_node_is_leaf(node)) {
      // We call the user's callback on all tris in this node that intersect. If there are
      // any non-zero return values, quit immediately
      for(uint32_t i = 0; i < node->faces_count; i++) {
        uint32_t faceidx = tree->nodefaces.array[i + node->faces_index];
        //logdebug("CHECK TRI %d", i);
        if(AABBCOLLIDE_3DBOX(box, &tree->faceboxes[faceidx])) {
          //logdebug("COLLIDE %d", i);
          int result = callback(state, tree->model, faceidx);
          if(result) return result;
        }
      }
    } else {
      //logdebug("DRILL DOWN");
      // Drill down to all the children
      for(int i = 0; i < 8; i++) {
        int result = octree_scan_recurse(
          tree, box, &tree->nodes.array[node->children_index + i], callback, state);
        if(result) return result;
      }
    }
  }
  return 0;
}

int octree_scan(octree * tree, collision_box_3d * box, octree_scan_callback callback, void * state) {
  // TODO: is root always 0? Why do we use rootidx above?
  return octree_scan_recurse(tree, box, &tree->nodes.array[0], callback, state);
}

