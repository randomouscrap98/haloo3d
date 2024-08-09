#ifndef HALOO3D_PRINT_H
#define HALOO3D_PRINT_H

#include "haloo3d.h"
#include <stdarg.h>
#include <stdint.h>
#include <string.h>

typedef struct {
  int x;
  int y;
  haloo3d_fb *fb;
  char *buffer;
  int buflen;
  uint16_t bcolor;
  uint16_t fcolor;
  uint64_t *glpyhs;
  // haloo3d_fb *font;
} haloo3d_print_tracker;

// Initialize the given print tracker to have all defaults. You will still need
// to pass some kind of char buffer for storing buffered prints. The default
// colors are chosen by the library (white text on black background)
void haloo3d_print_initdefault(haloo3d_print_tracker *t, char *buf, int buflen);

// print using the given tracker. Standard printf formatter
void haloo3d_print(haloo3d_print_tracker *t, const char *fmt, ...);

// reset the cursor and any other "temporary" tracking.
void haloo3d_print_refresh(haloo3d_print_tracker *t);

//{ snprintf((t)->buffer, (t)->buflen, fmt, __VA_ARGS__); }

void haloo3d_print_convertglyph(uint64_t glpyh, uint16_t bcolor,
                                uint16_t fcolor, haloo3d_fb *out);

// void haloo3d_print_charat(int x, int y, haloo3d_fb * fb,

// These are 8x8 glyphs for characters in the ascii range. If you try to
// print utf8, oh well
// clang-format off
const uint64_t haloo3d_print_basic_glyphs[256] = {
    143426159983824380,
    143426159983824380,
    143426159983824380,
    143426159983824380,
    143426159983824380,
    143426159983824380,
    143426159983824380,
    143426159983824380,
    143426159983824380,
    143426159983824380,
    0,
    143426159983824380,
    143426159983824380,
    143426159983824380,
    143426159983824380,
    143426159983824380,
    143426159983824380,
    143426159983824380,
    143426159983824380,
    143426159983824380,
    143426159983824380,
    143426159983824380,
    143426159983824380,
    143426159983824380,
    143426159983824380,
    143426159983824380,
    143426159983824380,
    143426159983824380,
    143426159983824380,
    143426159983824380,
    143426159983824380,
    143426159983824380,
    0,
    27286515379863648,
    61036401134665728,
    61358078227512536,
    9286067055097888,
    37520388433433992,
    63526353221029112,
    27127289393184768,
    15868979679617080,
    63103274824773856,
    60922667984748544,
    137992085536,
    1616912448,
    16252928,
    24672,
    2278395874476288,
    70260500491783416,
    67606977935585532,
    70241305799787004,
    70241254382603512,
    17008128708840460,
    143413665499811064,
    70242858578054392,
    143424798664475008,
    70242905822694648,
    70242905864637688,
    413927497824,
    413927497760,
    242411925560,
    4160813056,
    963955421408,
    70241254388007008,
    70243046284427512,
    31764399387413900,
    142300504155590136,
    70242856564002040,
    140061896669501936,
    143413158248284668,
    143413158248284544,
    71355563338730748,
    111901206737948044,
    70984677656113404,
    16901744509160696,
    111914555028904332,
    108510259257115132,
    111989647362329996,
    111971986726292876,
    70242908305722616,
    142300504154800512,
    70242908308871412,
    142300504157952396,
    70242854257921272,
    70984677656113200,
    111901204858899704,
    111901204847030304,
    111901344182144396,
    111988525359553932,
    57646073810792496,
    143020316906603004,
    70017728324354296,
    36240316375239684,
    69832285986756856,
    9131278913765376,
    508,
    18049582881570816,
    1065369832700,
    108510774855306744,
    1071820934392,
    3391982861651196,
    1071829057784,
    33884733781860448,
    1088992054520,
    108510774855306636,
    13511830482727420,
    3377751462350072,
    108510311206721932,
    108510259257073724,
    2171872325036,
    2171333348748,
    1071821720824,
    2171340423552,
    1088992054284,
    1897528394112,
    1088807505400,
    27129334340804732,
    1707476880632,
    1707465011232,
    1708015877336,
    2048572586460,
    1707465011680,
    2182790308348,
    15868567893336120,
    27127564814278752,
    63103378442432736,
    3987585024,
    143426159983824380,
    143426159983824380,
    143426159983824380,
    143426159983824380,
    143426159983824380,
    143426159983824380,
    143426159983824380,
    143426159983824380,
    143426159983824380,
    143426159983824380,
    143426159983824380,
    143426159983824380,
    143426159983824380,
    143426159983824380,
    143426159983824380,
    143426159983824380,
    143426159983824380,
    143426159983824380,
    143426159983824380,
    143426159983824380,
    143426159983824380,
    143426159983824380,
    143426159983824380,
    143426159983824380,
    143426159983824380,
    143426159983824380,
    143426159983824380,
    143426159983824380,
    143426159983824380,
    143426159983824380,
    143426159983824380,
    143426159983824380,
    143426159983824380,
    0,
    27022014123405408,
    9281723391866912,
    34002147725066748,
    150015967266816,
    111702070488266784,
    27127563203666016,
    67978378199436408,
    60798594969501696,
    70093270435562744,
    69820077355827200,
    467508713580,
    4162328576,
    0,
    70093133530334456,
    69805794224242688,
    70260501555707904,
    9043449317687544,
    70241442974138368,
    70241448007041024,
    4538783999459328,
    111901204865974656,
    35160083652945960,
    536870912,
    8256,
    67606977948942336,
    70242908295921664,
    1859056884144,
    36451647653413892,
    36451664563539996,
    108157396892517380,
    13511009092340984,
    18050067567869324,
    4539268685757836,
    9095644871261580,
    11347444684946828,
    60799079655800204,
    31613643008114060,
    35168642810360252,
    70242856756322368,
    18051771200012796,
    4540972317901308,
    9097348503405052,
    60800783287943676,
    18051766611374588,
    4540967729263100,
    9097343914766844,
    60800778699305468,
    67792373363300592,
    11348668551372172,
    18050654703291640,
    4539855821180152,
    9096232006683896,
    11348031820369144,
    60799666791222520,
    585459847304,
    70242977566264568,
    18051290358451448,
    4540491476339960,
    9096867661843704,
    60800302446382328,
    4540491466567792,
    108642252394199424,
    68004457988599224,
    18050648251403516,
    4539849369292028,
    9096225554795772,
    11348025368481020,
    60799660339334396,
    31614223691648252,
    2166508839416,
    278732656615488,
    18050654710628600,
    4539855828517112,
    9096232014020856,
    60799666798559480,
    18050614482186748,
    4539815600075260,
    9096191785579004,
    60799626570117628,
    13538276162768120,
    11349131331997068,
    18049587068374264,
    4538788186262776,
    9095164371766520,
    11346964185451768,
    237498698403064,
    137455206432,
    1072092269816,
    18049589551402232,
    4538790669290744,
    9095166854794488,
    237501181431032,
    4540491464471008,
    108510774862381440,
    60800302434513376
};
// clang-format on

#endif
