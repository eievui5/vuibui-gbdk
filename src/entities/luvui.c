#pragma bank 255

#include <gb/incbin.h>

#include "entities/luvui.h"
#include "include/entity.h"

INCBIN(gfx_luvui, res/gfx/sprites/luvui.h.2bpp)

const void __at(255) __bank_luvui;

const char luvui_metasprite[] = {
	// Idle.
	0, 0,
	2, 0,
	// Idle Flip.
	4, 0,
	6, 0,
	// Step
	8, 0,
	10, 0,
	// Step Flip.
	12, 0,
	14, 0,
};

const entity_data luvui_data = {
	.metasprites = luvui_metasprite,
	.graphics = gfx_luvui,
};