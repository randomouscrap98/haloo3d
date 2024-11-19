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
    printf("%16s X %6d = %5.2f ms\n", #func, repeat, time_spent * 1000);       \
    h3d_fb_writeppmfile(&fbn, #func ".ppm");                                   \
  }

#define FBDEFWIDTH 256
#define FBDEFHEIGHT 256

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
