#pragma once

#include "include/bank.h"
#include "include/int.h"

// VRAM allocation
#define CURSOR_TILE 0x00u
#define CURSOR_PALETTE 0u
#define BLANK_TILE 0x8Eu
#define FONT_TILE 0x8Fu
#define SUBFONT_TILE 0xA6u
#define PARTYFONT_TILE 0x35u
#define DESCFONT_TILE (SUBFONT_TILE + 56u)

// Animation speeds
#define CURSOR_SPEED 4u
#define SUBMENU_SLIDE_SPEED 8u
#define SWIPE_SPEED 12u

#define SUBMENU_SLIDE_POS 0x60u
#define SUBSUBMENU_SLIDE_POS 0x48u
#define NB_OPTIONS (6u - 1u)

BANK_EXTERN(pause_menu)
INCBIN_EXTERN(paw_cursor)

bool pause_menu() BANKED;