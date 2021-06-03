#include <gb/gb.h>
#include <stdint.h>

#include "include/entity.h"
#include "include/map.h"
#include "include/rendering.h"

uint8_t oam_index = 0;

/** Renders a 16 x 16 sprite based on the given entity's state.
 * 
 * @param target	A pointer to the target entity.
*/
void render_entity(entity *target, uint8_t x_off, uint8_t y_off) {
	// Y position
	shadow_OAM[oam_index].y = (target->y << 4) - camera_position.y + 16 + y_off;
	// X position
	shadow_OAM[oam_index].x = (target->x << 4) - camera_position.x + 8 + x_off;
	// Tile
	shadow_OAM[oam_index].tile = target->data->metasprite[0].tile_left;
	// Attributes
	shadow_OAM[oam_index++].prop = target->data->metasprite[0].attr_left;
	// Y position
	shadow_OAM[oam_index].y = (target->y << 4) - camera_position.y + 16 + y_off;
	shadow_OAM[oam_index].x = (target->x << 4) - camera_position.x + 16 + x_off;
	shadow_OAM[oam_index].tile = target->data->metasprite[0].tile_right;
	shadow_OAM[oam_index++].prop = target->data->metasprite[0].attr_right;
}

void render_movement(entity *target, uint8_t progress) {
	switch (target->direction) {
		case DIR_DOWN: {
			render_entity(target, 0, -progress);
			break;
		}
		case DIR_UP: {
			render_entity(target, 0, progress);
			break;
		}
		case DIR_RIGHT: {
			render_entity(target, -progress, 0);
			break;
		}
		case DIR_LEFT: {
			render_entity(target, progress, 0);
			break;
		}
	}
}