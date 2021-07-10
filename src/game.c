#pragma bank 255

#include <stdio.h>

#include "include/entity.h"
#include "include/hud.h"
#include "include/int.h"

#define TURNS_PER_MIN 15

extern u8 ignore_ally;

u8 cur_keys = 0;
u8 new_keys;
u8 rel_keys;
u8 last_keys;

u8 sub_mins = 0;
u8 minutes = 59;
u8 hours = 23;

// Handle ally and enemy AI
void do_turn() BANKED
{
	for (u8 i = 1; i < NB_ALLIES; i++) {
		if (i == ignore_ally) {
			ignore_ally = 0;
			continue;
		}
		if (entities.allies[i].data)
			pathfind(
				&entities.allies[i], entities.player.x_pos,
				entities.player.y_pos
			);
	}
	for (u8 i = 0; i < NB_ENEMIES; i++)
		if (entities.enemies[i].data)
			pursue(&entities.enemies[i], 0, 3);
	move_entities();

	if (!entities.player.data)
		while(1){wait_vbl_done();};

	if (++sub_mins >= TURNS_PER_MIN) {
		sub_mins = 0;
		if (++minutes >= 60) {
			minutes = 0;
			if (++hours >= 24)
				hours = 0;
		}
		draw_clock();
	}
}