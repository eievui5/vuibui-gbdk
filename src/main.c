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
		new_entity(
			&luvui_data, BANK(luvui),
			i, 28 + (i * 2), 32 + i, 4
		);
	}

	load_mapdata(&debug_mapdata, BANK(debug_mapdata));
	generate_map();
	move_entities();
	force_render_map();

	LCDC_REG = lcdc_buffer = \
		LCDC_ENABLE | \
		LCDC_BG_ENABLE | \
		LCDC_OBJ_ENABLE | \
		LCDC_OBJ_16;
	while(1) {
		update_input();

		if (cur_keys) {
			bool moved = false;
			if (new_keys & J_A) {
				char attack_msg[36] = "Luvui attacked!";
				vec8 pos = {
					entities.player.x_pos,
					entities.player.y_pos
				};
				move_direction(&pos, entities.player.direction);
				entity *target = check_entity_at(pos.x, pos.y);
				if (target) {
					if (target->health <= 1)
						memset(target, 0, sizeof(entity));
					else
						target->health -= 1;
				} else
					strcat(attack_msg, " Missed.");
				vwf_activate_font(0);
				vwf_wrap_str(20 * 8, attack_msg);
				print_hud(attack_msg);
				moved = true;
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