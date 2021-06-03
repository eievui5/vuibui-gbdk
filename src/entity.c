#include <gb/gb.h>
#include <rand.h>
#include <stdint.h>

#include "include/collision.h"
#include "include/entity.h"
#include "include/map.h"
#include "include/rendering.h"

uint8_t active_player = 0;
entity player_array[MAX_ALLIES];
entity entity_array[MAX_ENTITIES];

ACTION step_entity(entity *target) {
	switch (rand() & 0b11) {
		case DIR_DOWN: {
			if (get_collision(target->x, target->y + 1) == CLEAR_COLLISION) {
				target->direction = DIR_DOWN;
				target->y++;
				target->action = MOVE_ACTION;
				return MOVE_ACTION;
			}
			target->action = NO_ACTION;
			return NO_ACTION;
		}
		case DIR_UP: {
			if (get_collision(target->x, target->y - 1) == CLEAR_COLLISION) {
				target->direction = DIR_UP;
				target->y--;
				target->action = MOVE_ACTION;
				return MOVE_ACTION;
			}
			target->action = NO_ACTION;
			return NO_ACTION;
		}
		case DIR_RIGHT: {
			if (get_collision(target->x + 1, target->y) == CLEAR_COLLISION) {
				target->direction = DIR_RIGHT;
				target->x++;
				target->action = MOVE_ACTION;
				return MOVE_ACTION;
			}
			target->action = NO_ACTION;
			return NO_ACTION;
		}
		case DIR_LEFT: {
			if (get_collision(target->x - 1, target->y) == CLEAR_COLLISION) {
				target->direction = DIR_LEFT;
				target->x--;
				target->action = MOVE_ACTION;
				return MOVE_ACTION;
			}
			target->action = NO_ACTION;
			return NO_ACTION;
		}
	};

	return NO_ACTION;
}

void update_movement(void) {
	for (uint8_t progress = 16; progress > 0; progress--) {
		oam_index = 0;

		// Make the camera follow the player.
		if (PLAYER.action == MOVE_ACTION) {
			switch (PLAYER.direction) {
				case DIR_DOWN: {
					camera_position.y = (PLAYER.y << 4) - SCREENWIDTH/2 + 16 - progress;
					break;
				}
				case DIR_UP: {
					camera_position.y = (PLAYER.y << 4) - SCREENWIDTH/2 + 16 + progress;
					break;
				}
				case DIR_RIGHT: {
					camera_position.x = (PLAYER.x << 4) - SCREENHEIGHT/2 + 8 - progress;
					break;
				}
				case DIR_LEFT: {
					camera_position.x = (PLAYER.x << 4) - SCREENHEIGHT/2 + 8 + progress;
					break;
				}
			}
		}

		// Move players.
		for (uint8_t i = 0; i < MAX_ALLIES; i++) {
			if (player_array[i]) {
				if (player_array[i].action == MOVE_ACTION) {
					render_movement(&player_array[i], progress);
				} else {
					render_entity(&player_array[i], 0, 0);
				}
			}
		}

		// Move entities.
		for (uint8_t i = 0; i < MAX_ENTITIES; i++) {
			if (entity_array[i]) {
				if (entity_array[i].action == MOVE_ACTION) {
					render_movement(&entity_array[i], progress);
				} else {
					render_entity(&entity_array[i], 0, 0);
				}
			}
		}
		wait_vbl_done();
		SCX_REG = camera_position.x;
		SCY_REG = camera_position.y;
	}

	// Reset Players.
	for (uint8_t i = 0; i < MAX_ALLIES; i++) {
		if (player_array[i].action == MOVE_ACTION) {
			player_array[i].action = NO_ACTION;
		}
	}

	// Reset entities.
	for (uint8_t i = 0; i < MAX_ENTITIES; i++) {
		if (entity_array[i].action == MOVE_ACTION) {
			player_array[i].action = NO_ACTION;
		}
	}
}

// Runs one turn for each entity in the entity array.
void process_entity_array(void) {
	for (uint8_t i = 0; i < MAX_ENTITIES; i++) {
		if (entity_array[i]) {
			ACTION result = step_entity(&entity_array[i]);
			// Since walking can happen simultaneously buffer them
			// until another action is attempted.
			if (result > MOVE_ACTION) {
				update_movement();
			}
		}
	}
	update_movement();
}

// Renders all entities in the entity array.
void render_entity_array(void) {
	// Render Players.
	for (uint8_t i = 0; i < MAX_ALLIES; i++) {
		if (player_array[i]) { 
			render_entity(&player_array[i], 0, 0);
		}
	}

	// Render Entities.
	for (uint8_t i = 0; i < MAX_ENTITIES; i++) {
		if (entity_array[i]) { 
			render_entity(&entity_array[i], 0, 0);
		}
	}
}