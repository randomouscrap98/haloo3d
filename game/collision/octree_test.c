#include "collision.h"
#include "octree.h"
#include "../utils/test.h"
#include "../../haloo3d_ex.h"

// This also serves as our "basic triangle"
static const char onetri[1024] =
  "v -1.0 -1.0 -1.0\n"
  "v 1.0 -1.0 2.0\n"
  "v 0.0 3.0 0.0\n"
  "f 1 2 3\n";

// static const char eight[1024] =
//   "v -1.0 -1.0 -1.0\n"
//   "v 1.0 -1.0 2.0\n"
//   "v 0.0 3.0 0.0\n"
//   "f 1 2 3\n";

void octree_test() {
  // First test: just one big triangle
  h3d_obj obj;
  h3d_obj_loadstring(&obj, onetri, 256, 256);
  ASSERT(1, "h3d_obj_loadstring ontri");

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

  vec4 objvertex;

  // Now, let's add some more faces and regenerate. We're going to be generous
  // and put JUST ONE triangle in every quadrant
  for(int z = -1; z < 1; z++) {
    for(int y = -1; y < 1; y++) {
      for(int x = -1; x < 1; x++) {
        for(int i = 0; i < 3; i++) {
          VEC4(objvertex, x + 0.1, y + 0.2, z + 0.3, 1.0);
          h3d_obj_addvertex(&obj, objvertex);
        }
      }
    }
  }

  ASSERT(octree_init(&tree) == 0, "octree_init eighttri");
  ASSERT(octree_build(&tree, &obj) == 0, "octree_build eighttri");

  h3d_obj_free(&obj);
  ASSERT(1, "h3d_obj_free eighttri");
}
