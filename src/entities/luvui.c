#pragma bank 255

#include <gb/cgb.h>
#include <gb/incbin.h>
#include "include/entity.h"

#include "moves/scratch.h"
#include "moves/lunge.h"

INCBIN(gfx_luvui, res/gfx/sprites/luvui.h.2bpp)
INCBIN_EXTERN(gfx_luvui)

const void __at(255) __bank_luvui;

const entity_data luvui_entity = {
	.graphics = gfx_luvui,
	.colors = luvui_colors,
	.name = "Luvui",
	.base_xp = 64,
	.base_health = 70,
	.base_fatigue = 45,
	.base_attack = 58,
	.base_defense = 64,
	.level_moves = luvui_moves,
};

const short luvui_colors[] = {
	RGB_WHITE, RGB(0x1F, 0x14, 0x14), RGB(0x12, 0x04, 0x06), RGB(0x04, 0x00, 0x00)
};

const struct leveled_move luvui_moves[] = {
	{.level = 1, .bank = BANK(scratch), .data = &scratch_move},
	{.level = 3, .bank = BANK(lunge), .data = &lunge_move},
	{.level = 0xFF},
};