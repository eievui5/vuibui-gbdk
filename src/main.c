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
			&luvui_entity, BANK(luvui),
			i, 28 + (i * 2), 32, 4
		);
	}
	strcpy(entities.player.name, "Eievui");
	strcpy(entities.array[3].name, "Enemy");
	init_move_window();

	load_mapdata(&debug_mapdata, BANK(debug_mapdata));
	generate_map();
	move_entities();
	force_render_map();

	LCDC_REG = lcdc_buffer = \
		LCDC_ENABLE | LCDC_BG_ENABLE | LCDC_WINDOW_ENABLE | \
		LCDC_WINDOW_SCRN1 | LCDC_OBJ_ENABLE | LCDC_OBJ_16;
	while(1) {
		update_input();

		bool moved = false;
		static u8 window_bounce = 0;

		if (cur_keys & J_A) {
			// Handle window animation
			if (win_pos.x > 168 - 80 && window_bounce == 0) {
				win_pos.x -= 8;
			} else if (window_bounce != 2) {
				window_bounce = 1;
				if (win_pos.x < 168 - 72) {
					win_pos.x += 1;
				} else
					window_bounce = 2;
			}
			u8 selected = 255;
			if (new_keys & J_UP)
				selected = 0;
			if (new_keys & J_RIGHT)
				selected = 1;
			if (new_keys & J_DOWN)
				selected = 2;
			if (new_keys & J_LEFT) {
				selected = 3;
				init_hud();
			}
			if (selected != 255)
				if (entities.player.moves[selected].data) {
					win_pos.x = 168 - 72;
					window_bounce = 2;
					use_melee_move(
						&entities.player,
						&entities.player.moves[selected]
					);
					moved = true;
				}

		// If not holding the attack button, regress the window
		// and check for movement.
		} else {
			// Reset window if needed.
			if (win_pos.x < 168) {
				win_pos.x = 168;
				window_bounce = 0;
			}
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
		}
		
		if (moved)
			do_turn();
		else {
			render_entities();
			wait_vbl_done();
		}
		
	}
}