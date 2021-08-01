#pragma once

#include <gb/gb.h>
#include <stdint.h>

extern uint8_t cur_keys;
extern uint8_t new_keys;
extern uint8_t rel_keys;
extern uint8_t last_keys;

extern uint8_t sub_mins;
extern uint8_t minutes;
extern uint8_t hours;

void do_turn() BANKED;
void simulate_gameplay() BANKED;

inline void update_input()
{
	last_keys = cur_keys;
	cur_keys = joypad();
	new_keys = ~last_keys & cur_keys;
	rel_keys = last_keys & ~cur_keys;
}