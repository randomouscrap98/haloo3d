//#include "collision.h"
#include "collision.h"
#include "octree.h"
#include "../utils/test.h"
#include "../utils/log.h"
#include "../utils/print.h"
#include "../../haloo3d_ex.h"
#include "test_common.h"


typedef struct {
  h3d_obj * obj;
  uint32_t face;
} octrack_data;

VECTOR_DECLARE(octrack_data);
VECTOR_DEFINE(octrack_data);

int octrack_callback(void * state, h3d_obj * obj, uint32_t face) {
  vector_octrack_data * track = (vector_octrack_data *)state;
  size_t idx = 0;
  assert(vector_octrack_data_increment(track, &idx) == 0 && "octrack_callback");
  track->array[idx].obj = obj;
  track->array[idx].face = face;
  return 0;
}

void octree_test() {
  // First test: just one big triangle
  h3d_obj obj;
  h3d_obj_loadstring(&obj, onetri, 256, 256);
  ASSERT(1, "h3d_obj_loadstring ontri");

  char msg[256];

  // Now, let's save this one triangle for later use
  vec4 basetri[3];
  memcpy(basetri, obj.vertices, sizeof(vec4) * 3);

  // And the aabb
  //vec3 basetri_min, basetri_max;
  //collision_box_3d 
  //collision_objface_aabb(&obj, 0, basetri_min, basetri_max);

  octree tree;
  ASSERT(octree_init(&tree) == 0, "octree_init onetri");
  ASSERT(octree_build(&tree, &obj) == 0, "octree_build onetri");

  // With just one triangle, we should have one node and one tri pointer, which
  // points to the tri. I don't know, maybe too much inspection?
  ASSERT(tree.nodes.length == 1, "octree 1 node onetri");
  ASSERT(tree.nodefaces.length == 1, "octree 1 face onetri");
  assert(octree_node_is_leaf(tree.nodes.array) && "single node root is leaf onetri");

  // We also know the exact dimensions the octree is supposed to be
  octree_node * n = &tree.nodes.array[0];
  ASSERT(n->box.min[0] == -1.0, "octree xmin onetri");
  ASSERT(n->box.min[1] == -1.0, "octree ymin onetri");
  ASSERT(n->box.min[2] == -1.0, "octree zmin onetri");
  ASSERT(n->box.max[0] == 1.0, "octree xmax onetri");
  ASSERT(n->box.max[1] == 3.0, "octree ymax onetri");
  ASSERT(n->box.max[2] == 2.0, "octree zmax onetri");

  octree_free(&tree);
  ASSERT(1, "octree_free onetri");

  h3d_obj_free(&obj);
  ASSERT(1, "h3d_obj_free onetri");

  // Fully new test
  h3d_obj_init(&obj, 100, 100);

  // Now, let's add some more faces and regenerate. We're going to be generous
  // and put JUST ONE triangle in every quadrant
  eighttri(&obj);

  assert(octree_init(&tree) == 0 && "eighttri");
  assert(octree_build(&tree, &obj) == 0 && "eighttri");

  assert(tree.nodes.array[0].faces_count == 0 && "eighttri");
  printf("length %zu\n", tree.nodes.length);
  assert(tree.nodes.length == 9 && "eighttri");
  ASSERT(tree.nodefaces.length == 8, "octree 8 face eighttri (%zu)", tree.nodefaces.length);
  assert(!octree_node_is_leaf(tree.nodes.array) && "root not leaf eighttri");
  for(uint32_t i = 1; i < 9; i++) {
    snprintf(msg, 256, "octree one tri in %d "VEC3FMT(2)" -> "VEC3FMT(2), i, 
             VEC3SPREAD(tree.nodes.array[i].box.min), VEC3SPREAD(tree.nodes.array[i].box.max));
    ASSERT_EQ(1, tree.nodes.array[i].faces_count, "%u", msg);
    // Now, that ONE face should have an index that is the SAME as the index for triangle in the obj
    // (since everything should kinda just work out?). Not a good thing to assume most times
    assert(tree.nodes.array[i].faces_index == (i - 1) && "face index good");
    assert(octree_node_is_leaf(tree.nodes.array + i) && "leaf eighttri");
  }

  vector_octrack_data calltrack;
  vector_octrack_data_init(&calltrack);
  collision_box_3d box;
  // Very far out
  VEC3(box.min, -5, -5, -5);
  VEC3(box.max, -4, -4, -4);

  // Let's see how the callback functions (if at all)
  assert(octree_scan(&tree, &box, octrack_callback, &calltrack) == 0 && "eighttri");
  assert(calltrack.length == 0 && "eighttri");

  // And now let's get a collision with just ONE tri
  //VEC3(box.min, -1, -1, -1);
  //VEC3(box.max, -0.9, -0.9, -0.9); // this should be idx 0
  
  for(uint32_t fi = 0; fi < obj.numfaces; fi++) {
    //collision_objface_aabb(&obj, fi, &box);
    //logdebug("EIGHTTRI CALC BOUNDS[%d]: "VEC3FMT(1)" -> "VEC3FMT(1), fi, VEC3ARGS(box.min), VEC3ARGS(box.max));
    memcpy(box.min, obj.vertices[obj.faces[fi][0].verti], sizeof(vec3));
    memcpy(box.max, obj.vertices[obj.faces[fi][2].verti], sizeof(vec3));
    //hfloat_t * lowvert = obj.vertices[obj.faces[fi][0].verti];
    //hfloat_t * highvert = obj.vertices[obj.faces[fi][2].verti];
    logdebug("LOWVERT[%d]: "VEC3FMT(1), fi, VEC3ARGS(box.min));
    logdebug("HIGHVERT[%d]: "VEC3FMT(1), fi, VEC3ARGS(box.max));
    vector_octrack_data_clear(&calltrack);
    assert(octree_scan(&tree, &box, octrack_callback, &calltrack) == 0 && "eighttri");
    logdebug("intersect face %d count: %zu", fi, calltrack.length);
    assert(calltrack.length == 1 && "eighttri");
    // for(uint32_t vi = 0; vi < 3; vi++) {
    //   assert(box.min[vi] == lowvert[vi] && "eighttri");
    //   assert(box.max[vi] == highvert[vi] && "eighttri");
    // }
  }


  octree_free(&tree);
  ASSERT(1, "octree_free eighttri");

  h3d_obj_free(&obj);
  ASSERT(1, "h3d_obj_free eighttri");

  vector_octrack_data_free(&calltrack);
}
