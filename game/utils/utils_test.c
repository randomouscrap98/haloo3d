#include "utils.h"
#include "test.h"
#include "../../haloo3d.h"

void utils_test() {
  vec2 pos1, dim1, pos2, dim2;
  VEC2(pos1, 0, 0);
  VEC2(dim1, 10, 10);
  ASSERT(AABBCOLLIDE(pos1, dim1, pos1, dim1), "AABBCOLLIDE perfect overlap");
  VEC2(pos2, 2, 2);
  VEC2(dim2, 7, 7);
  ASSERT(AABBCOLLIDE(pos1, dim1, pos2, dim2), "AABBCOLLIDE full 1 overlap");
  ASSERT(AABBCOLLIDE(pos2, dim2, pos1, dim1), "AABBCOLLIDE full 2 overlap");
  VEC2(pos2, 9, 9);
  VEC2(dim2, 1, 1);
  ASSERT(AABBCOLLIDE(pos1, dim1, pos2, dim2), "AABBCOLLIDE barely overlap");
  VEC2(pos2, 10, 9);
  ASSERT(!AABBCOLLIDE(pos1, dim1, pos2, dim2), "AABBCOLLIDE barely miss x");
  VEC2(pos2, 9, 10);
  ASSERT(!AABBCOLLIDE(pos1, dim1, pos2, dim2), "AABBCOLLIDE barely miss y");
  // Now make sure things still work if every number is different and they BARELY collide. 4 corners.
  // This should be the ultimate test (?)
  VEC2(pos1, 1, 5);
  VEC2(dim1, 6, 8);
  // -- Barely clip the top left corner --
  VEC2(dim2, 2, 4);
  VEC2(pos2, 0, 2);
  ASSERT(AABBCOLLIDE(pos1, dim1, pos2, dim2), "AABBCOLLIDE top left corner");
  ASSERT(AABBCOLLIDE(pos2, dim2, pos1, dim1), "AABBCOLLIDE top left corner rev");
  // Unclip top left corner
  VEC2(pos2, -1, 2);
  ASSERT(!AABBCOLLIDE(pos1, dim1, pos2, dim2), "AABBCOLLIDE not top left corner x");
  ASSERT(!AABBCOLLIDE(pos2, dim2, pos1, dim1), "AABBCOLLIDE not top left corner x rev");
  VEC2(pos2, 0, 1);
  ASSERT(!AABBCOLLIDE(pos1, dim1, pos2, dim2), "AABBCOLLIDE not top left corner y");
  ASSERT(!AABBCOLLIDE(pos2, dim2, pos1, dim1), "AABBCOLLIDE not top left corner y rev");
  // -- Barely clip the bottom left corner --
  VEC2(pos2, 0, 12);
  ASSERT(AABBCOLLIDE(pos1, dim1, pos2, dim2), "AABBCOLLIDE bottom left corner");
  ASSERT(AABBCOLLIDE(pos2, dim2, pos1, dim1), "AABBCOLLIDE bottom left corner rev");
  // Unclip bottom left corner
  VEC2(pos2, -1, 12);
  ASSERT(!AABBCOLLIDE(pos1, dim1, pos2, dim2), "AABBCOLLIDE not bottom left corner x");
  ASSERT(!AABBCOLLIDE(pos2, dim2, pos1, dim1), "AABBCOLLIDE not bottom left corner x rev");
  VEC2(pos2, 0, 13);
  ASSERT(!AABBCOLLIDE(pos1, dim1, pos2, dim2), "AABBCOLLIDE not bottom left corner y");
  ASSERT(!AABBCOLLIDE(pos2, dim2, pos1, dim1), "AABBCOLLIDE not bottom left corner y rev");
  // -- Barely clip the top right corner --
  VEC2(pos2, 6, 2);
  ASSERT(AABBCOLLIDE(pos1, dim1, pos2, dim2), "AABBCOLLIDE top right corner");
  ASSERT(AABBCOLLIDE(pos2, dim2, pos1, dim1), "AABBCOLLIDE top right corner rev");
  // Unclip top right corner
  VEC2(pos2, 7, 2);
  ASSERT(!AABBCOLLIDE(pos1, dim1, pos2, dim2), "AABBCOLLIDE not top right corner x");
  ASSERT(!AABBCOLLIDE(pos2, dim2, pos1, dim1), "AABBCOLLIDE not top right corner x rev");
  VEC2(pos2, 6, 1);
  ASSERT(!AABBCOLLIDE(pos1, dim1, pos2, dim2), "AABBCOLLIDE not top right corner y");
  ASSERT(!AABBCOLLIDE(pos2, dim2, pos1, dim1), "AABBCOLLIDE not top right corner y rev");
  // -- Barely clip the bottom right corner --
  VEC2(pos2, 6, 12);
  ASSERT(AABBCOLLIDE(pos1, dim1, pos2, dim2), "AABBCOLLIDE bottom right corner");
  ASSERT(AABBCOLLIDE(pos2, dim2, pos1, dim1), "AABBCOLLIDE bottom right corner rev");
  // Unclip bottom right corner
  VEC2(pos2, 7, 12);
  ASSERT(!AABBCOLLIDE(pos1, dim1, pos2, dim2), "AABBCOLLIDE not bottom right corner x");
  ASSERT(!AABBCOLLIDE(pos2, dim2, pos1, dim1), "AABBCOLLIDE not bottom right corner x rev");
  VEC2(pos2, 6, 13);
  ASSERT(!AABBCOLLIDE(pos1, dim1, pos2, dim2), "AABBCOLLIDE not bottom right corner y");
  ASSERT(!AABBCOLLIDE(pos2, dim2, pos1, dim1), "AABBCOLLIDE not bottom right corner y rev");
}
