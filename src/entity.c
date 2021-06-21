#include <gb/gb.h>

#include "include/entity.h"
#include "include/int.h"
#include "include/rendering.h"

entity_array entities;

void move_entities()
{
	while(1) {
		u8 progress = 0;
		for (u8 i = 0; i < NB_ENTITIES; i++) {
			if (entities.array[i].data) {
				if ((entities.array[i].x_pos & 0xFF) * 16 != entities.array[i].x_spr) {
					if ((entities.array[i].x_pos & 0xFF) * 16 > entities.array[i].x_spr)
						entities.array[i].x_spr++;
					else
						entities.array[i].x_spr--;
				} else if ((entities.array[i].y_pos & 0xFF) * 16 != entities.array[i].y_spr) {
					if ((entities.array[i].y_pos & 0xFF) * 16 > entities.array[i].y_spr)
						entities.array[i].y_spr++;
					else
						entities.array[i].y_spr--;
				} else
					progress++;
			} else
				progress++;
		}
		if (progress == NB_ENTITIES)
			return;
		render_entities();
		clean_oam();
		wait_vbl_done();
	}
}

/**
 * Check for an entity at a given grid position.
 * 
 * @param ignore	Index of the calling entity, used to ignore self.
 * @param x		X position to check.
 * @param y		Y position to check.
 * 
 * @returns		The detected entity. NULL if no entity is found.
*/
entity *check_collision(u8 ignore, u16 x, u16 y)
{
	for (u8 i = 0; i < NB_ENTITIES; i++) {
		if (i == ignore)
			continue;
		if (entities.array[i].x_pos == x && entities.array[i].y_pos == y)
			return &entities.array[i];
	}
	return NULL;
}

void render_entities()
{
	for (u8 i = 0; i < NB_ENTITIES; i++) {
		if (entities.array[i].data) {
			shadow_OAM[oam_index].y = 16 + entities.array[i].y_spr;
			shadow_OAM[oam_index].x = 8 + entities.array[i].x_spr;
			shadow_OAM[oam_index].tile = entities.array[i].data->metasprites[0] + i * 2;
			shadow_OAM[oam_index].prop = entities.array[i].data->metasprites[1];
			oam_index++;
			shadow_OAM[oam_index].y = 16 + entities.array[i].y_spr;
			shadow_OAM[oam_index].x = 16 + entities.array[i].x_spr;
			shadow_OAM[oam_index].tile = entities.array[i].data->metasprites[2] + i * 2;
			shadow_OAM[oam_index].prop = entities.array[i].data->metasprites[3];
			oam_index++;
		}
	}
}