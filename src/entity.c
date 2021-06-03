#include <gb/gb.h>
#include <stdint.h>

#include "include/map.h"
#include "include/entity.h"
#include "include/rendering.h"

entity entity_array[MAX_ENTITIES];

/** Renders a 16 x 16 sprite based on the given entity's state.
 * 
 * @param target	A pointer to the target entity.
*/
void render_entity(entity *target) {
	// Y position
	shadow_OAM[oam_index].y = (target->y << 4) - camera_position.y + 16;
	// X position
	shadow_OAM[oam_index].x = (target->x << 4) - camera_position.x + 8;
	// Tile
	shadow_OAM[oam_index].tile = target->data->metasprite[target->frame].tile_left;
	// Attributes
	shadow_OAM[oam_index++].prop = target->data->metasprite[target->frame].attr_left;
	// Y position
	shadow_OAM[oam_index].y = (target->y << 4) - camera_position.y + 16;
	shadow_OAM[oam_index].x = (target->x << 4) - camera_position.x + 16;
	shadow_OAM[oam_index].tile = target->data->metasprite[target->frame].tile_right;
	shadow_OAM[oam_index++].prop = target->data->metasprite[target->frame].attr_right;
}

// Renders all entities in the entity array
void render_entity_array(void) {
	for (uint8_t i = 0; i < MAX_ENTITIES; i++) {
		if (entity_array[i]) { render_entity(&entity_array[i]); }
	}
}
