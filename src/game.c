#pragma bank 255

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "include/entity.h"
#include "include/hud.h"
#include "include/item.h"
#include "include/map.h"
#include "include/rendering.h"

#include "entities/luvui.h"

#define TURNS_PER_MIN 15
#define MINS_PER_SPAWN 5
#define ALLY_DETECTION 6

extern uint8_t ignore_ally;

uint8_t cur_keys = 0;
uint8_t new_keys;
uint8_t rel_keys;
uint8_t last_keys;

uint8_t sub_mins = 0;
uint8_t minutes = 59;
uint8_t hours = 23;

// Used to keep track of entity spawning. Increments once per game minute.
uint8_t spawn_ticker = 0;

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
			if (closest_entity)
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
	if (get_collision(PLAYER.x_pos, PLAYER.y_pos) == EXIT_COLL)
		create_new_floor();

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
			spawn_enemy(&luvui_entity, BANK(luvui));
		}
	}
}