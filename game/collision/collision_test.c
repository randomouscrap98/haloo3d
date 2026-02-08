#include "collision.h"
#include "../utils/test.h"
#include "../../haloo3d.h"

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
}
