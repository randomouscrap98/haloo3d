#include "collision.h"
#include "octree.h"
#include "../utils/test.h"
#include "../utils/print.h"
#include "../../haloo3d_ex.h"
#include "test_common.h"


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
  vec3 basetri_min, basetri_max;
  collision_objface_aabb(&obj, 0, basetri_min, basetri_max);

  octree tree;
  ASSERT(octree_init(&tree) == 0, "octree_init onetri");
  ASSERT(octree_build(&tree, &obj) == 0, "octree_build onetri");

  // With just one triangle, we should have one node and one tri pointer, which
  // points to the tri. I don't know, maybe too much inspection?
  ASSERT(tree.nodes.length == 1, "octree 1 node onetri");
  ASSERT(tree.faces.length == 1, "octree 1 face onetri");

  // We also know the exact dimensions the octree is supposed to be
  octree_node * n = &tree.nodes.array[0];
  ASSERT(n->min[0] == -1.0, "octree xmin onetri");
  ASSERT(n->min[1] == -1.0, "octree ymin onetri");
  ASSERT(n->min[2] == -1.0, "octree zmin onetri");
  ASSERT(n->max[0] == 1.0, "octree xmax onetri");
  ASSERT(n->max[1] == 3.0, "octree ymax onetri");
  ASSERT(n->max[2] == 2.0, "octree zmax onetri");

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
  //ASSERT_EQ(8, obj.numfaces, "%d", "octree obj faces eighttri");
  //ASSERT_EQ(24, obj.numvertices, "%d", "octree obj vertices eighttri");
  //ASSERT_EQ(0, octree_init(&tree), "%d", "octree_init eighttri");
  //ASSERT_EQ(0, octree_build(&tree, &obj), "%d", "octree_build eighttri");
  //ASSERT_EQ((size_t)9, tree.nodes.length, "%zu", "octree node eighttri");
  //ASSERT_EQ(0, tree.nodes.array[0].faces_count, "%u", "octree no tris in root");
  for(int i = 1; i < 9; i++) {
    snprintf(msg, 256, "octree one tri in %d (%.2f,%.2f,%.2f)-(%.2f,%.2f,%.2f)", i, 
             VEC3SPREAD(tree.nodes.array[i].min), VEC3SPREAD(tree.nodes.array[i].max));
    ASSERT_EQ(1, tree.nodes.array[i].faces_count, "%u", msg);
  }

  ASSERT(tree.faces.length == 8, "octree 8 face eighttri (%zu)", tree.faces.length);

  h3d_obj_free(&obj);
  ASSERT(1, "h3d_obj_free eighttri");
}
