#include <gb/cgb.h>
#include <gb/gb.h>
#include <rand.h>
#include <stdio.h>
#include <string.h>
#include "entities/luvui.h"
#include "include/dir.h"
#include "include/entity.h"
#include "include/hardware.h"
#include "include/hud.h"
#include "include/input.h"
#include "include/int.h"
#include "include/map.h"
#include "include/rendering.h"
#include "libs/vwf.h"
#include "mapdata/debug_mapdata.h"

u8 cur_keys = 0;
u8 new_keys;
u8 rel_keys;
u8 last_keys;

const char test_string[] = \
"Luvui used Attack. It's super effective! Critical hit! Enemy took 65535 \
damage and was defeated.";

void main()
{
	if (_cpu == CGB_TYPE) {
		cpu_fast();
		cgb_compatibility(); // Temporarily init first two pals to grey.
	}
	wait_vbl_done();
	LCDC_REG = 0;
	add_VBL(&vblank);
	set_interrupts(VBL_IFLAG | LCD_IFLAG);
	STAT_REG = STATF_LYC;
	BGP_REG = 0b11100100;
	OBP0_REG = 0b11010000;
	OBP1_REG = 0b11100100;
	init_hud();
	print_hud("Auto-tiling! This selects from a\nfew connecting tiles to stitch\ntogether the visible tile map.");
	initrand(742);
	memset(&entities, 0, sizeof(entity) * NB_ENTITIES);
	for (u8 i = 0; i < 4; i++) {
		entities.array[i].data = &luvui_data;
		entities.array[i].bank = BANK(luvui);
		entities.array[i].x_pos = 28 + (i * 2);
		entities.array[i].y_pos = 32 + i;
		entities.array[i].x_spr = (32 + i) * 16;
		entities.array[i].y_spr = (32 + i) * 16;
		SWITCH_ROM_MBC1(entities.array[i].bank);
		set_sprite_data(
			i * NB_ENTITY_TILES, NB_ENTITY_TILES,
			entities.array[i].data->graphics
		);
		if (_cpu == CGB_TYPE)
			set_sprite_palette(i, 1, entities.array[i].data->colors);
	}

	load_mapdata(&debug_mapdata, BANK(debug_mapdata));
	generate_map();
	move_entities();
	force_render_map();

	lcdc_buffer = LCDC_ENABLE | LCDC_BG_ENABLE | LCDC_OBJ_ENABLE | LCDC_OBJ_16;
	LCDC_REG = LCDC_ENABLE | LCDC_BG_ENABLE | LCDC_OBJ_ENABLE | LCDC_OBJ_16;
	while(1) {
		update_input();

		if (cur_keys & (J_DOWN | J_UP | J_LEFT | J_RIGHT)) {
			bool moved = false;
			if (cur_keys & J_DOWN) {
				entities.player.direction = DIR_DOWN;
				moved = player_try_step();
			}
			else if (cur_keys & J_UP) {
				entities.player.direction = DIR_UP;
				moved = player_try_step();
			}
			else if (cur_keys & J_RIGHT) {
				entities.player.direction = DIR_RIGHT;
				moved = player_try_step();
			}
			else if (cur_keys & J_LEFT) {
				entities.player.direction = DIR_LEFT;
				moved = player_try_step();
			}
			
			if (moved) {
				do_turn();
			}
		}
		
		render_entities();
		wait_vbl_done();
	}
}