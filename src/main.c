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
#include "include/int.h"
#include "include/map.h"
#include "include/rendering.h"
#include "mapdata/debug_mapdata.h"

u8 cur_keys = 0;
u8 new_keys;
u8 rel_keys;
u8 last_keys;

void main()
{
	if (_cpu == CGB_TYPE) {
		cpu_fast();
		cgb_compatibility(); // Temporarily init first two pals to grey.
	}
	wait_vbl_done();
	LCDC_REG = LCDC_WINDOW_SCRN1;
	add_VBL(&vblank);
	add_LCD(&show_hud);
	set_interrupts(VBL_IFLAG | LCD_IFLAG);
	LYC_REG = SCREENHEIGHT - 33;
	STAT_REG = STATF_LYC;
	BGP_REG = 0b11100100;
	OBP0_REG = 0b11010000;
	OBP1_REG = 0b11100100;
	init_hud();
	initrand(0);
	memset(&entities, 0, sizeof(entity) * NB_ENTITIES);
	for (u8 i = 0; i < 1; i++) {
		entities.array[i].data = &luvui_data;
		entities.array[i].bank = bank_luvui;
		entities.array[i].x_pos = 32 + i;
		entities.array[i].y_pos = 32 + i;
		entities.array[i].x_spr = (32 + i) * 16;
		entities.array[i].y_spr = (32 + i) * 16;
		SWITCH_ROM_MBC1(entities.array[i].bank);
		set_sprite_data(
			i * NB_ENTITY_TILES, NB_ENTITY_TILES,
			entities.array[i].data->graphics
		);
	}

	load_mapdata(&debug_mapdata, bank_debug_mapdata);
	generate_map();
	move_entities();
	force_render_map();
	
	lcdc_buffer = LCDC_ENABLE | LCDC_BG_ENABLE | LCDC_OBJ_ENABLE | LCDC_OBJ_16;
	LCDC_REG = LCDC_ENABLE | LCDC_BG_ENABLE | LCDC_OBJ_ENABLE | LCDC_OBJ_16;
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
				char buf[MESSAGE_SIZE];
				sprintf(
					buf,
					"Current Position:\n%u, %u", 
					entities.player.x_pos, 
					entities.player.y_pos
				);
				print_hud(buf);
			}
		}
		
		render_entities();
		wait_vbl_done();
	}
}