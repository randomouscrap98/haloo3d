#ifndef H3D_FLOWER
// F = flower color
// S = stem color
// D = disk color (usually yellow)
// B = bottom flower color (darker than F)
// Y = bottom stem color (darker than S)
// clang-format off
#define H3D_FLOWER(f, s, d, b, y) { \
  0,0,f,0,0,f,0,0, \
  0,0,f,d,d,f,0,0, \
  0,0,f,f,f,b,0,0, \
  0,0,s,b,b,y,0,0, \
  0,0,0,s,y,0,0,0, \
  0,s,s,s,y,y,y,0, \
  0,0,0,s,y,0,0,0, \
  0,0,0,s,y,0,0,0, \
}
// clang-format on
#endif
