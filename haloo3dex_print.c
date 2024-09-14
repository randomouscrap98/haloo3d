
#include "haloo3dex_print.h"

// These are 8x8 glyphs for characters in the ascii range. If you try to
// print utf8, oh well
// clang-format off
const uint64_t haloo3d_print_basic_glyphs[256] = {
	9179274238964760320,
	9179274238964760320,
	9179274238964760320,
	9179274238964760320,
	9179274238964760320,
	9179274238964760320,
	9179274238964760320,
	9179274238964760320,
	9179274238964760320,
	9179274238964760320,
	0,
	9179274238964760320,
	9179274238964760320,
	9179274238964760320,
	9179274238964760320,
	9179274238964760320,
	9179274238964760320,
	9179274238964760320,
	9179274238964760320,
	9179274238964760320,
	9179274238964760320,
	9179274238964760320,
	9179274238964760320,
	9179274238964760320,
	9179274238964760320,
	9179274238964760320,
	9179274238964760320,
	9179274238964760320,
	9179274238964760320,
	9179274238964760320,
	9179274238964760320,
	9179274238964760320,
	0,
	864704451948973056,
	77916943872,
	3926917006560802304,
	594308291526264832,
	2546271121760928256,
	4495563687961759232,
	17314876416,
	4038609588785526784,
	1015614699495493120,
	907818496,
	578780756331986944,
	290495421644537856,
	68169720922112,
	868068828175663104,
	72908667983241216,
	4496672031474138624,
	9086038739982491136,
	9152165585887641088,
	4495542665477307904,
	6944690690302179328,
	4495542819536207616,
	4495545975336812032,
	217874930004623104,
	4495545972652457472,
	4495542948995481088,
	868068879916597248,
	579838503764885504,
	4043122079833260032,
	17451714844033024,
	1016749309024010240,
	864704581909822976,
	4468480587830869504,
	7161707897292200960,
	4567603570985352960,
	4495440164522507776,
	2248249917505216256,
	9152162299476410112,
	217020638773346048,
	9107232251455962624,
	7161677231228674816,
	9086038739982515712,
	4495546118630832128,
	7148086785261921024,
	9152162179217294080,
	7161677145800401664,
	7161694806436438784,
	4495546131566247424,
	217020777829187328,
	6787904730176896512,
	7148086992631447296,
	4495542672506961408,
	1736164148113866240,
	4495546131566256896,
	584401852148441856,
	7161699221595448064,
	7167265623011451648,
	1736164304046417408,
	9153300282022592256,
	4469266303152438784,
	4638760496030679552,
	4481134612758674944,
	426109569024,
	9151314442816847872,
	525312,
	9107261823295422464,
	4567603724993889024,
	4495440577822720000,
	9107232150446432256,
	4468555319500341248,
	868082075986312192,
	4494731393896546304,
	7161677110359294720,
	9158096334119507968,
	4495542820089651200,
	7148091339132044032,
	8650855245695615744,
	7740398492928966656,
	7161677110359097344,
	4495546130938986496,
	217086902535192576,
	6944689591186096128,
	217020948950286336,
	4566789675129241600,
	8938532608234228736,
	4495546131559743488,
	584401852141928448,
	3926975509056978944,
	8592336193679523840,
	1088805010407424000,
	9155286121465249792,
	4038616220315695104,
	868082074056920064,
	1015588345173511680,
	16725143816503296,
	9179274238964760320,
	9179274238964760320,
	9179274238964760320,
	9179274238964760320,
	9179274238964760320,
	9179274238964760320,
	9179274238964760320,
	9179274238964760320,
	9179274238964760320,
	9179274238964760320,
	9179274238964760320,
	9179274238964760320,
	9179274238964760320,
	9179274238964760320,
	9179274238964760320,
	9179274238964760320,
	9179274238964760320,
	9179274238964760320,
	9179274238964760320,
	9179274238964760320,
	9179274238964760320,
	9179274238964760320,
	9179274238964760320,
	9179274238964760320,
	9179274238964760320,
	9179274238964760320,
	9179274238964760320,
	9179274238964760320,
	9179274238964760320,
	9179274238964760320,
	9179274238964760320,
	9179274238964760320,
	9179274238964760320,
	0,
	873168492155636736,
	594029897647523840,
	9153010024071969792,
	9601021905076224,
	593921263713805056,
	868082022517312512,
	4350616204763930112,
	13824,
	4485969256336408064,
	138965787098624,
	7797449725392715776,
	13563841728217088,
	0,
	4485942902282862080,
	15872,
	68613771050496,
	4467579661845006336,
	138590950735360,
	68595868712448,
	528384,
	217086903145685760,
	2893611337457236992,
	34359738368,
	290482175965396992,
	138641948548608,
	68596590132736,
	1960873667403448320,
	4629831612383822336,
	8074997297135682048,
	4629827222927311616,
	4495440280832448512,
	7169558215846790144,
	7169558215846793216,
	7169558215847577600,
	7169558215847585792,
	7169558215846278656,
	7169558215847582720,
	8870718504632744960,
	290550770944916992,
	9152192967623574528,
	9152192967623577600,
	9152192967624361984,
	9152192967623063040,
	9159226650880640000,
	9159226650880643072,
	9159226650881427456,
	9159226650880128512,
	2177040098522439168,
	7166189523864659968,
	4495546130939511808,
	4495546130939514880,
	4495546130940299264,
	4495546130940307456,
	4495546130939000320,
	2455596579386556416,
	4495550564240932352,
	4495546131560268800,
	4495546131560271872,
	4495546131561056256,
	4495546131559757312,
	2025562528994037760,
	234014984164541184,
	4281624976256343552,
	9107261823295947776,
	9107261823295950848,
	9107261823296735232,
	9107261823296743424,
	9107261823295436288,
	9107261823296740352,
	4543427701976858624,
	290620727373987840,
	4468555319500866560,
	4468555319500869632,
	4468555319501654016,
	4468555319500355072,
	9158096334120027136,
	9158096334120030208,
	9158096334120814592,
	9158096334119515648,
	4495546132015880192,
	7161677110360418304,
	4495545970985534464,
	4495545970985537536,
	4495545970986321920,
	4495545970986330112,
	4495545970988548096,
	576528922158563328,
	4496680895658328064,
	4495546129899324416,
	4495546129899327488,
	4495546129900111872,
	4495546129902338048,
	1088805010407952384,
	217086902535389952,
	1088805010407437824,
};
// clang-format on

void haloo3d_print_initdefault(haloo3d_print_tracker *t, char *buf,
                               int buflen) {
  t->scale = 1;
  t->bcolor = 0xF000;
  t->fcolor = 0xFFFF;
  t->glyphs = haloo3d_print_basic_glyphs;
  t->buffer = buf;
  t->buflen = buflen;
  t->logprints = 0;
  haloo3d_print_refresh(t);
  // NOTE: don't forget to set fb
}

void haloo3d_print_refresh(haloo3d_print_tracker *t) {
  t->x = 0;
  t->y = 0;
}

void haloo3d_print(haloo3d_print_tracker *t, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  vsnprintf(t->buffer, t->buflen, fmt, args);
  va_end(args);
  if (t->logprints) {
    eprintf("%s", t->buffer);
  }
  haloo3d_fb tex;
  uint8_t prerendered[256] = {0};
  //   32k on the stack! is this ok??
  uint16_t buffer[H3D_PRINT_CHW * H3D_PRINT_CHH * 256];
  tex.buffer = buffer;
  tex.width = H3D_PRINT_CHW;
  tex.height = H3D_PRINT_CHH;
  haloo3d_recti srect = {.x1 = 0,
                         .y1 = 0,
                         .x2 = t->scale * H3D_PRINT_CHW,
                         .y2 = t->scale * H3D_PRINT_CHH};
  haloo3d_recti trect = {
      .x1 = 0, .y1 = 0, .x2 = H3D_PRINT_CHW, .y2 = H3D_PRINT_CHH};
  haloo3d_presprite presprite = haloo3d_sprite_precalc(t->fb, trect, srect);
  const int len = strlen(t->buffer);
  for (int i = 0; i < len; i++) {
    if (t->buffer[i] == '\n') {
      t->y += t->scale * H3D_PRINT_CHH;
      t->x = 0;
      continue;
    }
    // We don't support word wrap btw
    int glyph = t->buffer[i];
    tex.buffer = buffer + glyph * H3D_PRINT_CHW * H3D_PRINT_CHH;
    if (!prerendered[glyph]) {
      haloo3d_print_convertglyph(t->glyphs[glyph], t->bcolor, t->fcolor, &tex);
      prerendered[glyph] = 1;
    }
    srect.x1 = t->x;
    srect.y1 = t->y;
    srect.x2 = srect.x1 + t->scale * H3D_PRINT_CHW;
    srect.y2 = srect.y1 + t->scale * H3D_PRINT_CHH;
    // haloo3d_sprite(t->fb, &tex, trect, srect);
    haloo3d_sprite_withprecalc(&tex, &presprite, srect);
    t->x += t->scale * H3D_PRINT_CHW;
  }
}

void haloo3d_print_convertglyph(uint64_t glyph, uint16_t bcolor,
                                uint16_t fcolor, haloo3d_fb *out) {
  for (int i = 0; i < H3D_PRINT_CHW * H3D_PRINT_CHH; i++) {
    out->buffer[i] = (glyph & 1) ? fcolor : bcolor;
    glyph >>= 1;
  }
}
