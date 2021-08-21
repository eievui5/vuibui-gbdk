#pragma bank 255

#include <gb/cgb.h>
#include <gb/incbin.h>
#include "include/entity.h"

#include "moves/scratch.h"
#include "moves/lunge.h"

INCBIN(gfx_forest_rat, res/gfx/sprites/rat.h.2bpp)
INCBIN_EXTERN(gfx_forest_rat)

const void __at(255) __bank_forest_rat;

const entity_data forest_rat_entity = {
	.graphics = gfx_forest_rat,
	.colors = forest_rat_colors,
	.name = "Forest Rat",
	.base_xp = 64,
	.base_health = 64,
	.base_fatigue = 45,
	.base_attack = 32,
	.base_defense = 28,
	.level_moves = forest_rat_moves,
};

const short forest_rat_colors[] = {
	RGB_WHITE, RGB(16, 22, 12), RGB(10, 10, 2), RGB(2, 3, 0)
};

const struct leveled_move forest_rat_moves[] = {
	{.level = 1, .bank = BANK(scratch), .data = &scratch_move},
	{.level = 0xFF},
};