#pragma bank 255

#include <gb/incbin.h>

#include "include/item.h"

const void __at(255) __bank_apple;
INCBIN(apple_graphic, res/gfx/items/apple.2bpp)
INCBIN_EXTERN(apple_graphic)
const char apple_name[] = "Apple";
const char apple_desc[] = \
"A small red fruit.\nEat it to restore\nyour health and\nenergy.";
const item_data apple_item = {
	.name = apple_name,
	.graphic = apple_graphic,
	.desc = apple_desc,
};