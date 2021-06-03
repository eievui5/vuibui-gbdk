#pragma once

#include <stdint.h>
#include "vector.h"

#define MAX_ALLIES 3
#define MAX_ENTITIES 5

#define PLAYER player_array[active_player]

typedef enum {
	NO_ACTION,
	MOVE_ACTION
} ACTION;

typedef struct {
	uint8_t tile_left;
	unsigned char attr_left;
	uint8_t tile_right;
	unsigned char attr_right;
} entity_frame;

/** Structure for defining constant data for entities. These definitions should
 * be in a common bank.
 * 
 * @param bank	The bank where the following data is stored.
 * @param metasprite	An array of animation frames.
*/
typedef struct {
	// The bank containing all of the entity data.
	const unsigned char bank;
	// An array of 4-byte metasprite frames.
	// Since entities are always 16x16, the entity metasprite format
	// excludes x and y.
	const entity_frame *metasprite;
} entity_data;

/** Structure for active entities.
 * 
 * @param data	A pointer to the entity's constant data. If this is set to null 
 * the entity is considered inactive.
 * @param x	16x16 grid-locked 16-bit X position.
 * @param y	16x16 grid-locked 16-bit Y position.
 * @param action
 * @param direction	Used to determine which direction the entity is facing.
 * @param frame	Used to index into the entity's metasprite array to display a
 * given frame.
*/
typedef struct {
	const entity_data *data;
	uint16_t x, y;
	ACTION action;
	uint8_t direction;
} entity;

extern uint8_t active_player;
extern entity player_array[MAX_ALLIES];
extern entity entity_array[MAX_ENTITIES];

void process_entity_array(void);
void render_entity_array(void);