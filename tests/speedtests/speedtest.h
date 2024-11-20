// Each test in this folder should be a c file that imports this header.
// Each test will set up its own data, then call the macro which
// creates a loop to run your program and output results to
// stdout in a standard format. This will also write out
// the ppm file for the final "scene".

// These tests do not assume the structure of anything

// Loop the given function for the given amount of times, passing the
// given arguments to it (including the fb). It will print the run info
// to stdout, and write the fb out
#define SPEEDTESTLOOP(func, repeat, fbn, ...)                                  \
  {                                                                            \
    eprintf("Starting %s...\n", #func);                                        \
    H3D_FB_FILL(&fbn, 0xF000);                                                 \
    clock_t begin = clock();                                                   \
    for (int _i = 0; _i < repeat; ++_i) {                                      \
      func(&fbn, __VA_ARGS__);                                                 \
    }                                                                          \
    clock_t end = clock();                                                     \
    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;                \
    printf("%24s * %6d = %5.2f ms\n", #func, repeat, time_spent * 1000);       \
    h3d_fb_writeppmfile(&fbn, #func ".ppm");                                   \
  }

#define FBDEFWIDTH 256
#define FBDEFHEIGHT 256
#define DEFFOV 90.0
#define DEFNEAR 0.01
#define DEFFAR 100.0
#define RF3DDEFUP {0.0, 1.0, 0.0}
#define RF3DDEFPOS {-0.75, 0.5, 1.1}
#define RF3DDEFLOOK {0.0, 0.0, 0.0}
#define RF3DDEFVECS                                                            \
  {{0.0, 1.0, 0.0, 1.0}, {-1.0, -1.0, 0.0, 1.0}, {1.0, -1.0, 0.0, 1.0}}

// Create a framebuffer, pre-cleared, of the default size.
#define DEFAULTFB_UNIGI(fbn)                                                   \
  h3d_fb fbn;                                                                  \
  h3d_fb_init(&fbn, FBDEFWIDTH, FBDEFHEIGHT);                                  \
  H3D_FB_FILL(&fbn, 0xF000);

// Create a default rasterface which takes up some reasonable amount of
// space on the default fb screen. 3D is not used or assumed, and
// the interpolants are just u and v.
#define DEFAULT_RASTERFACE2D(rfn)                                              \
  h3d_rasterface rfn;                                                          \
  rfn[0].pos[H3DX] = FBDEFWIDTH / 2;                                           \
  rfn[0].pos[H3DY] = FBDEFHEIGHT / 16;                                         \
  rfn[0].interpolants[0] = 0.5;                                                \
  rfn[0].interpolants[1] = 1.0;                                                \
  rfn[1].pos[H3DX] = FBDEFWIDTH / 16;                                          \
  rfn[1].pos[H3DY] = FBDEFHEIGHT * 15 / 16;                                    \
  rfn[1].interpolants[0] = 0;                                                  \
  rfn[1].interpolants[1] = 0;                                                  \
  rfn[2].pos[H3DX] = FBDEFWIDTH * 15 / 16;                                     \
  rfn[2].pos[H3DY] = FBDEFHEIGHT * 15 / 16;                                    \
  rfn[2].interpolants[0] = 1.0;                                                \
  rfn[2].interpolants[1] = 0;

// Create a default rasterface which takes up some reasonable amount of space
// on the default fb screen. Interpolants are z, u, and v. The triangle is
// rendered at an angle to enable testing of depth and such
#define DEFAULT_RASTERFACE3D(rfn)                                              \
  h3d_rasterface rfn;                                                          \
  {                                                                            \
    /* Setup premade projection/translation/etc matrix */                      \
    vec3 up = RF3DDEFUP;                                                       \
    vec3 pos = RF3DDEFPOS;                                                     \
    vec3 lookat = RF3DDEFLOOK;                                                 \
    mat4 cammatrix, perspective, finalmat;                                     \
    h3d_perspective(DEFFOV, (hfloat_t)FBDEFWIDTH / FBDEFHEIGHT, DEFNEAR,       \
                    DEFFAR, perspective);                                      \
    h3d_my_lookat(pos, lookat, up, cammatrix);                                 \
    mat4_inverse(cammatrix, cammatrix);                                        \
    mat4_multiply(perspective, cammatrix, finalmat);                           \
    /* Setup 3dface in 3d space translated with previous matrix */             \
    vec4 verts[3] = RF3DDEFVECS;                                               \
    h3d_3dface face3d;                                                         \
    for (int _i = 0; _i < 3; _i++) {                                           \
      h3d_vec4_mult_mat4(verts[_i], finalmat, face3d[_i].pos);                 \
    }                                                                          \
    if (!h3d_3dface_normalize(face3d)) {                                       \
      dieerr("TRIFACE NOT FACING CAMERA???");                                  \
    }                                                                          \
    /* Setup renderface by translating to screen space + set interpolants */   \
    for (int _v = 0; _v < 3; _v++) {                                           \
      rfn[_v].interpolants[0] = face3d[_v].pos[H3DW];                          \
      h3d_viewport(face3d[_v].pos, FBDEFWIDTH, FBDEFHEIGHT, rfn[_v].pos);      \
    }                                                                          \
    rfn[0].interpolants[1] = 0.5;                                              \
    rfn[0].interpolants[2] = 1.0;                                              \
    rfn[1].interpolants[1] = 0;                                                \
    rfn[1].interpolants[2] = 0;                                                \
    rfn[2].interpolants[1] = 1.0;                                              \
    rfn[2].interpolants[2] = 0;                                                \
  }
