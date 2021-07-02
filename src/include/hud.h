#include <gb/incbin.h>

#define MESSAGE_SIZE 54

INCBIN_EXTERN(hud_tiles)
INCBIN_EXTERN(font_tiles)
INCBIN_EXTERN(health_tiles)

void init_hud() BANKED;
void show_hud() NONBANKED;
void print_hud(const char *src) BANKED;