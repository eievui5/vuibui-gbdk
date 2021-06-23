#include <gb/gb.h>
#include <rand.h>
#include <string.h>

#include "include/bank.h"
#include "include/entity.h"
#include "include/hardware.h"
#include "include/rendering.h"

#include "../res/gfx/sprites/gfx_luvui.h"

const char debug_metasprite[] = {
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

const entity_data debug_entity_data = {
	.metasprites = debug_metasprite,
	.graphics = gfx_luvui,
	.gfx_bank = bank_gfx_luvui
};

u8 cur_keys = 0;
u8 new_keys;
u8 rel_keys;
u8 last_keys;

void lcd_memcpy(size_t len, const unsigned char *src, unsigned char *dest) NONBANKED;

void main()
{
	initrand(0);
	LCDC_REG = LCDC_ENABLE | LCDC_BG_ENABLE | LCDC_OBJ_ENABLE | LCDC_OBJ_16;
	memset(&entities, 0, sizeof(entity) * NB_ENTITIES);
	for (u8 i = 0; i < 1; i++) {
		entities.array[i].data = &debug_entity_data;
		entities.array[i].x_pos = 1 + i;
		entities.array[i].y_pos = 1 + i;
		SET_BANK(bank_gfx_luvui);
		set_sprite_data(i * NB_ENTITY_TILES, NB_ENTITY_TILES, gfx_luvui);
	}
	move_entities();
	while(1) {
		last_keys = cur_keys;
		cur_keys = joypad();
		new_keys = ~last_keys & cur_keys;
		rel_keys = last_keys & ~cur_keys;

		if (cur_keys & (J_DOWN | J_UP | J_LEFT | J_RIGHT)) {
			bool moved = false;
			if (cur_keys & J_DOWN) {
				moved = try_step(0, DIR_DOWN);
			}
			else if (cur_keys & J_UP) {
				moved = try_step(0, DIR_UP);
			}
			else if (cur_keys & J_RIGHT) {
				moved = try_step(0, DIR_RIGHT);
			}
			else if (cur_keys & J_LEFT) {
				moved = try_step(0, DIR_LEFT);
			}
			
			if (moved) {
				for (u8 i = 1; i < NB_ENTITIES; i++) {
					if (entities.array[i].data)
						try_step(i, rand() & 0b11);
				}
				move_entities();
			}
		}

		render_entities();
		clean_oam();
		wait_vbl_done();
	}
}