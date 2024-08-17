#include "../haloo3dex_console.h"

int main() {
  int value_1 = 5;
  int value_2 = 10;
  uint16_t value_3 = 65500;
  // You MUST have some kind of storage for strings to be usable in the system,
  // otherwise people can't set it!
  char name[64] = "Wow, it's a string!!";

  haloo3d_debugconsole dc;
  haloo3d_debugconsole_init(&dc);

  haloo3d_debugconsole_set(&dc, "value_1.int", (void *)&value_1);
  haloo3d_debugconsole_set(&dc, "area/value_2.int", (void *)&value_2);
  haloo3d_debugconsole_set(&dc, "value_3.uint16", (void *)&value_3);
  haloo3d_debugconsole_set(&dc, "area/value_1_again.int", (void *)&value_1);
  haloo3d_debugconsole_set(&dc, "area/deeper/name.str", (void *)name);

  haloo3d_debugconsole_beginprompt(&dc);
}
