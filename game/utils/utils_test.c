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
}
