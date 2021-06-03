#include <stdint.h>

#include "include/collision.h"
#include "include/entity.h"

// Checks both the map and entity arrays to see if the target location is valid.
COLLISION get_collision(uint16_t x, uint16_t y) {
	for (uint8_t i = 0; i < MAX_ALLIES; i++) {
		if (player_array[i]) {
			if (player_array[i].x == x && player_array[i].y == y) {
				return WALL_COLLISION;
			}
		}
	}
	for (uint8_t i = 0; i < MAX_ENTITIES; i++) {
		if (entity_array[i]) {
			if (entity_array[i].x == x && entity_array[i].y == y) {
				return WALL_COLLISION;
			}
		}
	}
	return CLEAR_COLLISION;
}