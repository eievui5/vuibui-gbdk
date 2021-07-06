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
#include "include/vec.h"
#include "libs/vwf.h"
#include "mapdata/debug_mapdata.h"

u8 cur_keys = 0;
u8 new_keys;
u8 rel_keys;
u8 last_keys;

void main()
{
	if (_cpu == CGB_TYPE) {
		cpu_fast();
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
	initrand(742);
	memset(&entities, 0, sizeof(entity) * NB_ENTITIES);
	for (u8 i = 0; i < 4; i += 3) {
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

		if (cur_keys) {
			bool moved = false;
			if (new_keys & J_A) {
				char attack_msg[] = "Luvui attacked!";
				vec8 pos = {
					entities.player.x_pos,
					entities.player.y_pos
				};
				move_direction(&pos, entities.player.direction);
				entity *target = check_entity_at(pos.x, pos.y);
				if (target)
					strcat(attack_msg, " Hit enemy.");
				else
					strcat(attack_msg, " Missed.");
				vwf_activate_font(0);
				vwf_wrap_str(20 * 8, attack_msg);
				print_hud(attack_msg);
			}
			else if (cur_keys & J_DOWN) {
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