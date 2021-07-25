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
	.metasprites = luvui_metasprite,
	.graphics = gfx_luvui,
	.colors = luvui_colors,
	.name = luvui_name,
	.base_xp = 255,
	.base_health = 64,
	.base_fatigue = 45,
	.level_moves = luvui_moves,
};

const char luvui_metasprite[] = {
	// Idle.
	0, 0,
	2, 0,
	// Idle Flip.
	4, 0,
	6, 0,
	// Step.
	8, 0,
	10, 0,
	// Step Flip.
	12, 0,
	14, 0,
};

const short luvui_colors[] = {
	RGB_WHITE, RGB(0x1F, 0x14, 0x14), RGB(0x12, 0x04, 0x06), RGB(0x04, 0x00, 0x00)
};

const char luvui_name[] = "Luvui";

const struct leveled_move luvui_moves[] = {
	{.level = 1, .bank = BANK(scratch), .data = &scratch_move},
	{.level = 3, .bank = BANK(lunge), .data = &lunge_move},
	{.level = 0xFF},
};