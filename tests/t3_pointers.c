#include "../haloo3d.h"
#include <stdio.h>
#include <string.h>

void printvec4(vec4 v) {
  printf("Test: %f, %f, %f, %f\n", v[0], v[1], v[2], v[3]);
  printf("V is %zu bytes\n", sizeof(v));
}

int main() {
  vec4 test;
  test[H3DX] = 0.5;
  test[H3DY] = 0.75;
  test[H3DZ] = -1.0;
  test[H3DW] = 1.0;
  printvec4(test);
  float_t test2[4];
  memcpy(test2, test, sizeof(vec4));
  printvec4(test2);
  float_t *test3 = test2;
  printvec4(test3);
}
