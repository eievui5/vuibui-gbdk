#pragma once

#include <gb/gb.h>
#include <stdint.h>

enum GameState {DUNGEON_STATE, WORLDMAP_STATE};

extern enum GameState game_state;

extern uint8_t cur_keys;
extern uint8_t new_keys;
extern uint8_t rel_keys;
extern uint8_t last_keys;

extern uint8_t sub_mins;
extern uint8_t minutes;
extern uint8_t hours;

extern uint8_t current_floor;

void do_turn() BANKED;
void game_loop() BANKED;

inline void update_input()
{
	last_keys = cur_keys;
	cur_keys = joypad();
	new_keys = ~last_keys & cur_keys;
	rel_keys = last_keys & ~cur_keys;
}