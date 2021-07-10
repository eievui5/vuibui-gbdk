#pragma once

#include "include/int.h"

extern u8 cur_keys;
extern u8 new_keys;
extern u8 rel_keys;
extern u8 last_keys;

extern u8 sub_mins;
extern u8 minutes;
extern u8 hours;

void do_turn() BANKED;

inline void update_input()
{
	last_keys = cur_keys;
	cur_keys = joypad();
	new_keys = ~last_keys & cur_keys;
	rel_keys = last_keys & ~cur_keys;
}