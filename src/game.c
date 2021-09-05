#pragma bank 255

#include <rand.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "include/bank.h"
#include "include/cutscene.h"
#include "include/dialogue.h"
#include "include/dir.h"
#include "include/entity.h"
#include "include/game.h"
#include "include/hud.h"
#include "include/item.h"
#include "include/map.h"
#include "include/rendering.h"
#include "include/world.h"
#include "menus/pause.h"
#include "menus/title.h"

#include "entities/forest_rat.h"

#define TURNS_PER_MIN 15
#define MINS_PER_SPAWN 5
#define ALLY_DETECTION 6

DEF_BANK(gamebank)
BANK_EXTERN(gamebank)

extern uint8_t ignore_ally;

enum GameState game_state;

uint8_t cur_keys = 0;
uint8_t new_keys;
uint8_t rel_keys;
uint8_t last_keys;

uint8_t sub_mins = 0;
uint8_t minutes = 59;
uint8_t hours = 23;

uint8_t current_floor;

// Used to keep track of entity spawning. Increments once per game minute.
uint8_t spawn_ticker = 0;

// Generate the next floor or exit the dungeon.
void exit_floor() NONBANKED
{
	uint8_t temp_bank = _current_bank;
	SWITCH_ROM_MBC1(current_mapdata_bank);
	if (current_floor == current_mapdata->final_floor) {
		set_sram_flag(current_mapdata->completion_flag);
		game_state = WORLDMAP_STATE;
	} else {
		current_floor++;
		swipe_left(true);
		create_new_floor();
		swipe_right();
	}
	SWITCH_ROM_MBC1(temp_bank);
}

// Handle ally and enemy AI
void do_turn() BANKED
{
	for (uint8_t i = 1; i < NB_ALLIES; i++) {
		if (i == ignore_ally) {
			ignore_ally = 0;
			continue;
		}
		if (entities[i]) {
			uint8_t closest_entity = 0;
			uint8_t closest_distance = 6;
			for (uint8_t j = 3; j < NB_ENTITIES; j++) {
				uint8_t distance = abs(entities[i].x_pos - entities[j].x_pos) + abs(entities[i].y_pos - entities[j].y_pos);
				if (distance < closest_distance) {
					closest_distance = distance;
					closest_entity = j;
				}
			}
			if (closest_entity &&
			    detection_trace(entities[i].x_pos,
			    		    entities[i].y_pos,
			    		    entities[closest_entity].x_pos,
			    		    entities[closest_entity].y_pos, 4))
				pursue(&entities[i], closest_entity, closest_entity + 1);
			else if (i == 2 && entities[1])
				ally_pathfind(&entities[i], entities[1].x_pos, entities[1].y_pos);
			else
				ally_pathfind(&entities[i], PLAYER.x_pos, PLAYER.y_pos);
		}
	}
	for (uint8_t i = BEGIN_ENEMIES; i < NB_ENTITIES; i++)
		if (entities[i].data)
			pursue(&entities[i], 0, 3);
	move_entities();

	if (!PLAYER.data)
		while(1) wait_vbl_done();

	for (uint8_t i = 0; i < NB_WORLD_ITEMS; i++)
		if (world_items[i].data)
			if (PLAYER.x_pos == world_items[i].x && 
			    PLAYER.y_pos == world_items[i].y)
				pickup_item(i);
	if (get_collision(PLAYER.x_pos, PLAYER.y_pos) == EXIT_COLL) {
		exit_floor();
	}

	if (++sub_mins >= TURNS_PER_MIN) {
		sub_mins = 0;
		if (++minutes >= 60) {
			minutes = 0;
			if (++hours >= 24)
				hours = 0;
		}
		draw_clock();

		if ((uint8_t)(spawn_ticker++ >= MINS_PER_SPAWN)) {
			spawn_ticker = 0;
			generate_enemy();
		}
	}
}

void init_dungeon() BANKED {
	current_floor = 0;
	init_hud(); 
	initrand(DIV_REG);
	swipe_left(false);
	reload_mapdata();
	create_new_floor();
	for (uint8_t i = 0; i < NB_ENTITIES; i++) {
		if (entities[i].data) {
			reload_entity_graphics(i);
		}
	}
	swipe_right();
}

void simulate_dungeon() BANKED {
	static uint8_t window_bounce = 0;
	static uint8_t speedup_delay = 0;
	bool moved = false;

	if (new_keys == J_START && cur_keys == J_START) {
		if(pause_menu())
			do_turn();
		return;
	}

	// Waiting and running checks.
	if (new_keys == J_SELECT)
		moved = true;
	if ((cur_keys & J_B) && !(cur_keys & J_A) &&
	(cur_keys & (J_UP | J_DOWN | J_LEFT | J_RIGHT)))
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
		uint8_t selected = 255;
		if (new_keys & J_UP)
			selected = 0;
		else if (new_keys & J_RIGHT)
			selected = 1;
		else if (new_keys & J_DOWN)
			selected = 2;
		else if (new_keys & J_LEFT) {
			selected = 3;
		}
		if (selected != 255)
			if (PLAYER.moves[selected].data) {
				win_pos.x = 168;
				window_bounce = 0;
				use_melee_move(&PLAYER, &PLAYER.moves[selected]);
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
			moved = ally_try_step(&PLAYER);
		}
		else if (cur_keys & J_UP) {
			PLAYER.direction = DIR_UP;
			moved = ally_try_step(&PLAYER);
		}
		else if (cur_keys & J_RIGHT) {
			PLAYER.direction = DIR_RIGHT;
			moved = ally_try_step(&PLAYER);
		}
		else if (cur_keys & J_LEFT) {
			PLAYER.direction = DIR_LEFT;
			moved = ally_try_step(&PLAYER);
		}
	}
	if (moved)
		do_turn();
	else {
		render_entities();
	}
}

// The game loop should *not* return. 
// Sub-loops, while being useful, are an *anti-pattern* and cause issues when 
// switching between states.
// Please prefer states in the game loop rather than while loops!
void game_loop() BANKED {
	while (1) {
		static enum GameState prev_game_state = 255;
		// Handle new game states.
		if (prev_game_state != game_state) {
			// Exit prev state.
			switch (prev_game_state) {

			}
			reset_oam();
			// Init new state.
			switch (game_state) {
			case DUNGEON_STATE:
				init_dungeon();
				break;
			case WORLDMAP_STATE:
				init_worldmap();
				break;
			case TITLESCREEN_STATE:
				init_title();
				break;
			case CUTSCENE_STATE:
				init_cutscene();
				break;
			}
			prev_game_state = game_state;
		}

		// Run state logic.
		switch (game_state) {
		case DUNGEON_STATE:
			simulate_dungeon();
			break;
		case WORLDMAP_STATE:
			simulate_worldmap();
			break;
		case TITLESCREEN_STATE:
			simulate_title();
			break;
		case CUTSCENE_STATE:
			simulate_cutscene();
			break;
		};
		wait_vbl_done();
	}
}