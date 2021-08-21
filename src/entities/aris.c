#pragma bank 255

#include <gb/cgb.h>
#include <gb/incbin.h>

#include "include/bank.h"
#include "include/entity.h"

#include "moves/scratch.h"
#include "moves/lunge.h"

INCBIN(gfx_aris, res/gfx/sprites/aris.h.2bpp)
INCBIN_EXTERN(gfx_aris)
DEF_BANK(aris)

const entity_data aris_entity = {
	.graphics = gfx_aris,
	.colors = aris_colors,
	.name = "Aris",
	.base_xp = 64,
	.base_health = 58,
	.base_fatigue = 45,
	.base_attack = 70,
	.base_defense = 64,
	.level_moves = aris_moves,
};

const short aris_colors[] = {
	RGB_WHITE, RGB(31, 31, 16), RGB(4, 4, 22), RGB(0, 0, 4)
};

const struct leveled_move aris_moves[] = {
	{.level = 1, .bank = BANK(scratch), .data = &scratch_move},
	{.level = 3, .bank = BANK(lunge), .data = &lunge_move},
	{.level = 0xFF},
};