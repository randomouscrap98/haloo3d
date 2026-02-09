#include "octree.h"
#include "../utils/test.h"
#include "../../haloo3d_ex.h"

static const char onetri[1024] =
  "v -1.0 -1.0 -1.0\n"
  "v 1.0 -1.0 2.0\n"
  "v 0.0 3.0 0.0\n"
  "f 1 2 3\n";

void octree_test() {
  // First test: just one big triangle
  h3d_obj obj;
  h3d_obj_loadstring(&obj, onetri, 256, 256);
  ASSERT(1, "h3d_obj_loadstring ontri");

  octree tree;
  ASSERT(octree_init(&tree) == 0, "octree_init onetri");
  ASSERT(octree_build(&tree, &obj) == 0, "octree_build onetri");

  octree_free(&tree);
  ASSERT(1, "octree_free onetri");

  h3d_obj_free(&obj);
  ASSERT(1, "h3d_obj_free onetri");
}
