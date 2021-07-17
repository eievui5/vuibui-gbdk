#include <gb/cgb.h>
#include <gb/gb.h>
#include <rand.h>
#include <stdio.h>
#include <string.h>

#include "include/dir.h"
#include "include/entity.h"
#include "include/game.h"
#include "include/hardware.h"
#include "include/hud.h"
#include "include/int.h"
#include "include/item.h"
#include "include/map.h"
#include "include/rendering.h"
#include "include/vec.h"
#include "libs/vwf.h"

#include "entities/luvui.h"
#include "items/apple.h"
#include "mapdata/debug_mapdata.h"
#include "menus/pause.h"
#include "moves/lunge.h"

void main()
{
	if (_cpu == CGB_TYPE)
		cpu_fast();
	wait_vbl_done();
	LCDC_REG = 0;
	fx_mode = GAME_UI;
	add_VBL(&vblank);
	set_interrupts(VBL_IFLAG | LCD_IFLAG);
	STAT_REG = STAT_LYC;
	BGP_REG = 0b11100100;
	OBP0_REG = 0b11010000;
	OBP1_REG = 0b11100100;
	init_hud();
	initrand(7894);
	memset(entities, 0, sizeof(entities));
	memset(world_items, 0, sizeof(world_items));
	memset(inventory, 0, sizeof(inventory));
	new_entity(&luvui_entity, BANK(luvui), 0, 32, 32, 4);
	PLAYER.health = 65535;
	strcpy(PLAYER.name, "Eievui");

	current_mapdata = &debug_mapdata;
	current_mapdata_bank = BANK(debug_mapdata);
	reload_mapdata();
	generate_map();
	for (u8 i = 0; i < NB_WORLD_ITEMS; i++) {
		world_items[i].data = &apple_item;
		world_items[i].bank = BANK(apple);
		world_items[i].x = 30 + i;
		world_items[i].y = 30;
		world_items[i].palette = 6;
	}
	load_item_graphics();
	move_entities();
	force_render_map();

	LCDC_REG = lcdc_buffer = \
		LCDC_ENABLE | LCDC_BG_ENABLE | LCDC_WINDOW_ENABLE | \
		LCDC_WINDOW_SCRN1 | LCDC_OBJ_ENABLE | LCDC_OBJ_16;
	while(1) {
		update_input();

		bool moved = false;
		static u8 window_bounce = 0;
		static u8 speedup_delay = 0;

		if (new_keys == J_START && cur_keys == J_START) {
			pause_menu();
			continue;
		}

		// Waiting and running checks.
		if (new_keys == J_SELECT)
			moved = true;
		if (
			(cur_keys & J_B) && !(cur_keys & J_A) &&
			(cur_keys & (J_UP | J_DOWN | J_LEFT | J_RIGHT))
		)
			move_speed = 2;
		else
			move_speed = 1;

		if ((cur_keys & J_A) && (cur_keys & J_B)) {
			if ((new_keys & J_A) || (new_keys & J_B)) {
				win_pos.x = 168;
				window_bounce = 0;
				draw_dir_window();
			}
			// Handle window animation
			if (win_pos.x > 168 - 48 && window_bounce == 0) {
				win_pos.x -= 8;
			} else if (window_bounce != 2) {
				window_bounce = 1;
				if (win_pos.x < 168 - 40) {
					win_pos.x += 1;
				} else
					window_bounce = 2;
			}
			if (new_keys & J_UP)
				PLAYER.direction = DIR_UP;
			else if (new_keys & J_RIGHT)
				PLAYER.direction = DIR_RIGHT;
			else if (new_keys & J_DOWN)
				PLAYER.direction = DIR_DOWN;
			else if (new_keys & J_LEFT)
				PLAYER.direction = DIR_LEFT;
			vset(0x9C42, 0x8A + PLAYER.direction);

		// Attack check.
		} else if (cur_keys & J_A) {
			if ((new_keys & J_A) || (last_keys & J_B)) {
				//win_pos.x = 168;
				window_bounce = 0;
				draw_move_window();
			}
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
			else if (new_keys & J_RIGHT)
				selected = 1;
			else if (new_keys & J_DOWN)
				selected = 2;
			else if (new_keys & J_LEFT) {
				selected = 3;
				PLAYER.moves[0].data = &lunge_move;
			}
			if (selected != 255)
				if (PLAYER.moves[selected].data) {
					win_pos.x = 168;
					window_bounce = 0;
					use_melee_move(
						&PLAYER,
						&PLAYER.moves[selected]
					);
					moved = true;
				}

		// If not holding the attack button, hide the window
		// and check for movement.
		} else {
			// Reset window if needed.
			if (win_pos.x < 168) {
				win_pos.x = 168;
				window_bounce = 0;
			}
			if (cur_keys & J_DOWN) {
				PLAYER.direction = DIR_DOWN;
				moved = player_try_step();
			}
			else if (cur_keys & J_UP) {
				PLAYER.direction = DIR_UP;
				moved = player_try_step();
			}
			else if (cur_keys & J_RIGHT) {
				PLAYER.direction = DIR_RIGHT;
				moved = player_try_step();
			}
			else if (cur_keys & J_LEFT) {
				PLAYER.direction = DIR_LEFT;
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