#include "collision.h"
#include "test_common.h"
#include "../utils/test.h"
#include "../utils/log.h"
#include "../../haloo3d.h"
#include "../../haloo3d_ex.h"

void collision_test() {
  vec2 pos1, dim1, pos2, dim2;
  VEC2(pos1, 0, 0);
  VEC2(dim1, 10, 10);
  ASSERT(AABBCOLLIDE_2DDIM(pos1, dim1, pos1, dim1), "AABBCOLLIDE_2DDIM perfect overlap");
  VEC2(pos2, 2, 2);
  VEC2(dim2, 7, 7);
  ASSERT(AABBCOLLIDE_2DDIM(pos1, dim1, pos2, dim2), "AABBCOLLIDE_2DDIM full 1 overlap");
  ASSERT(AABBCOLLIDE_2DDIM(pos2, dim2, pos1, dim1), "AABBCOLLIDE_2DDIM full 2 overlap");
  VEC2(pos2, 9, 9);
  VEC2(dim2, 1, 1);
  ASSERT(AABBCOLLIDE_2DDIM(pos1, dim1, pos2, dim2), "AABBCOLLIDE_2DDIM barely overlap");
  VEC2(pos2, 10, 9);
  ASSERT(!AABBCOLLIDE_2DDIM(pos1, dim1, pos2, dim2), "AABBCOLLIDE_2DDIM barely miss x");
  VEC2(pos2, 9, 10);
  ASSERT(!AABBCOLLIDE_2DDIM(pos1, dim1, pos2, dim2), "AABBCOLLIDE_2DDIM barely miss y");
  // Now make sure things still work if every number is different and they BARELY collide. 4 corners.
  // This should be the ultimate test (?)
  VEC2(pos1, 1, 5);
  VEC2(dim1, 6, 8);
  // -- Barely clip the top left corner --
  VEC2(dim2, 2, 4);
  VEC2(pos2, 0, 2);
  ASSERT(AABBCOLLIDE_2DDIM(pos1, dim1, pos2, dim2), "AABBCOLLIDE_2DDIM top left corner");
  ASSERT(AABBCOLLIDE_2DDIM(pos2, dim2, pos1, dim1), "AABBCOLLIDE_2DDIM top left corner rev");
  // Unclip top left corner
  VEC2(pos2, -1, 2);
  ASSERT(!AABBCOLLIDE_2DDIM(pos1, dim1, pos2, dim2), "AABBCOLLIDE_2DDIM not top left corner x");
  ASSERT(!AABBCOLLIDE_2DDIM(pos2, dim2, pos1, dim1), "AABBCOLLIDE_2DDIM not top left corner x rev");
  VEC2(pos2, 0, 1);
  ASSERT(!AABBCOLLIDE_2DDIM(pos1, dim1, pos2, dim2), "AABBCOLLIDE_2DDIM not top left corner y");
  ASSERT(!AABBCOLLIDE_2DDIM(pos2, dim2, pos1, dim1), "AABBCOLLIDE_2DDIM not top left corner y rev");
  // -- Barely clip the bottom left corner --
  VEC2(pos2, 0, 12);
  ASSERT(AABBCOLLIDE_2DDIM(pos1, dim1, pos2, dim2), "AABBCOLLIDE_2DDIM bottom left corner");
  ASSERT(AABBCOLLIDE_2DDIM(pos2, dim2, pos1, dim1), "AABBCOLLIDE_2DDIM bottom left corner rev");
  // Unclip bottom left corner
  VEC2(pos2, -1, 12);
  ASSERT(!AABBCOLLIDE_2DDIM(pos1, dim1, pos2, dim2), "AABBCOLLIDE_2DDIM not bottom left corner x");
  ASSERT(!AABBCOLLIDE_2DDIM(pos2, dim2, pos1, dim1), "AABBCOLLIDE_2DDIM not bottom left corner x rev");
  VEC2(pos2, 0, 13);
  ASSERT(!AABBCOLLIDE_2DDIM(pos1, dim1, pos2, dim2), "AABBCOLLIDE_2DDIM not bottom left corner y");
  ASSERT(!AABBCOLLIDE_2DDIM(pos2, dim2, pos1, dim1), "AABBCOLLIDE_2DDIM not bottom left corner y rev");
  // -- Barely clip the top right corner --
  VEC2(pos2, 6, 2);
  ASSERT(AABBCOLLIDE_2DDIM(pos1, dim1, pos2, dim2), "AABBCOLLIDE_2DDIM top right corner");
  ASSERT(AABBCOLLIDE_2DDIM(pos2, dim2, pos1, dim1), "AABBCOLLIDE_2DDIM top right corner rev");
  // Unclip top right corner
  VEC2(pos2, 7, 2);
  ASSERT(!AABBCOLLIDE_2DDIM(pos1, dim1, pos2, dim2), "AABBCOLLIDE_2DDIM not top right corner x");
  ASSERT(!AABBCOLLIDE_2DDIM(pos2, dim2, pos1, dim1), "AABBCOLLIDE_2DDIM not top right corner x rev");
  VEC2(pos2, 6, 1);
  ASSERT(!AABBCOLLIDE_2DDIM(pos1, dim1, pos2, dim2), "AABBCOLLIDE_2DDIM not top right corner y");
  ASSERT(!AABBCOLLIDE_2DDIM(pos2, dim2, pos1, dim1), "AABBCOLLIDE_2DDIM not top right corner y rev");
  // -- Barely clip the bottom right corner --
  VEC2(pos2, 6, 12);
  ASSERT(AABBCOLLIDE_2DDIM(pos1, dim1, pos2, dim2), "AABBCOLLIDE_2DDIM bottom right corner");
  ASSERT(AABBCOLLIDE_2DDIM(pos2, dim2, pos1, dim1), "AABBCOLLIDE_2DDIM bottom right corner rev");
  // Unclip bottom right corner
  VEC2(pos2, 7, 12);
  ASSERT(!AABBCOLLIDE_2DDIM(pos1, dim1, pos2, dim2), "AABBCOLLIDE_2DDIM not bottom right corner x");
  ASSERT(!AABBCOLLIDE_2DDIM(pos2, dim2, pos1, dim1), "AABBCOLLIDE_2DDIM not bottom right corner x rev");
  VEC2(pos2, 6, 13);
  ASSERT(!AABBCOLLIDE_2DDIM(pos1, dim1, pos2, dim2), "AABBCOLLIDE_2DDIM not bottom right corner y");
  ASSERT(!AABBCOLLIDE_2DDIM(pos2, dim2, pos1, dim1), "AABBCOLLIDE_2DDIM not bottom right corner y rev");

  // See if we even calculate the right bounds for the basic onetri
  vec3 trimin, trimax;
  h3d_obj obj;
  h3d_obj_loadstring(&obj, onetri, 256, 256);
  collision_objface_aabb(&obj, 0, trimin, trimax);
  logdebug("ONETRI CALC BOUNDS: "VEC3FMT(1)" -> "VEC3FMT(1), VEC3ARGS(trimin), VEC3ARGS(trimax));
  assert(trimin[0] == -1.0 && "onetri");
  assert(trimin[1] == -1.0 && "onetri");
  assert(trimin[2] == -1.0 && "onetri");
  assert(trimax[0] == 1.0 && "onetri");
  assert(trimax[1] == 3.0 && "onetri");
  assert(trimax[2] == 2.0 && "onetri");
  h3d_obj_free(&obj);

  h3d_obj_init(&obj, 100, 100);
  eighttri(&obj);
  for(uint32_t fi = 0; fi < obj.numfaces; fi++) {
    collision_objface_aabb(&obj, fi, trimin, trimax);
    logdebug("EIGHTTRI CALC BOUNDS[%d]: "VEC3FMT(1)" -> "VEC3FMT(1), fi, VEC3ARGS(trimin), VEC3ARGS(trimax));
    hfloat_t * lowvert = obj.vertices[obj.faces[fi][0].verti];
    hfloat_t * highvert = obj.vertices[obj.faces[fi][2].verti];
    logdebug("LOWVERT[%d]: "VEC3FMT(1), fi, VEC3ARGS(lowvert));
    logdebug("HIGHVERT[%d]: "VEC3FMT(1), fi, VEC3ARGS(highvert));
    for(uint32_t vi = 0; vi < 3; vi++) {
      assert(trimin[vi] == lowvert[vi] && "eighttri");
      assert(trimax[vi] == highvert[vi] && "eighttri");
    }
  }
  h3d_obj_free(&obj);
}
