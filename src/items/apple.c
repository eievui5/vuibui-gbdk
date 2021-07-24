#pragma bank 255

#include <gb/cgb.h>
#include <gb/incbin.h>

#include "include/item.h"

const void __at(255) __bank_apple;
INCBIN(apple_graphic, res/gfx/items/apple.2bpp)
INCBIN_EXTERN(apple_graphic)
const char apple_name[] = "Apple";
const char apple_desc[] = \
"A small red fruit.\nEat it to restore\nyour health and\nenergy.";
const short apple_palette[] = {
	RGB(10, 0x18, 12), RGB(31, 0, 0), RGB(16, 0, 0), RGB(8, 0, 0)
};
const healitem_data apple_item = {
	.data = {
		.name = apple_name,
		.graphic = apple_graphic,
		.desc = apple_desc,
		.type = HEAL_ITEM,
		.colors = apple_palette,
	},
	.health = 50,
};