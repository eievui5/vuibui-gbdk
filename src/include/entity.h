#pragma once

#include <stdint.h>
#include "vector.h"

// This shouldn't go over 20, as there are only 40 sprite slots and all entities
// should be able to be displayed in a single screen.
#define MAX_ENTITIES 16

typedef struct{
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
 * @param x	16x16 grid-locked 16-bit X position
 * @param y	16x16 grid-locked 16-bit Y position
 * @param frame	Used to index into the entity's metasprite array to display a
 * given frame.
*/
typedef struct {
	entity_data *data;
	uint16_t x, y;
	uint8_t frame;
} entity;

extern entity entity_array[MAX_ENTITIES];

void render_entity_array(void);