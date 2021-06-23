#include <gb/gb.h>
#include <stdbool.h>

#include "include/bank.h"
#include "include/entity.h"
#include "include/int.h"
#include "include/rendering.h"

#include "../res/gfx/sprites/gfx_luvui.h"

u8 anim_timer = 0;
entity_array entities;

bool try_step(u8 i, u8 dir)
{
	entities.array[i].direction = dir;
	u16 target_x = entities.array[i].x_pos;
	u16 target_y = entities.array[i].y_pos;
	switch (dir) {
	case DIR_DOWN:
		target_y++;
		break;
	case DIR_UP:
		target_y--;
		break;
	case DIR_RIGHT:
		target_x++;
		break;
	case DIR_LEFT:
		target_x--;
		break;
	}
	if (!check_collision(i, target_x, target_y)) {
		entities.array[i].x_pos = target_x;
		entities.array[i].y_pos = target_y;
		return true;
	}
	return false;
}

void move_entities()
{
	while(1) {
		u8 progress = 0;
		for (u8 i = 0; i < NB_ENTITIES; i++) {
			if (entities.array[i].data) {
				entities.array[i].spr_frame = 8;
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
				} else {
					entities.array[i].spr_frame = 0;
					progress++;
				}
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
		if (!entities.array[i].data)
			continue;
		if (entities.array[i].x_pos == x && entities.array[i].y_pos == y)
			return &entities.array[i];
	}
	return NULL;
}

/**
 * Renders two 8x16 sprites at each entity's position. The tile is chosen based
 * on the entity's index, allowing each entity to stream its own graphics.
*/
void render_entities()
{
	for (u8 i = 0; i < NB_ENTITIES; i++) {
		if (entities.array[i].data) {

			// Update the entity's graphics if needed.
			if (entities.array[i].direction != entities.array[i].prev_dir) {
				entities.array[i].prev_dir = entities.array[i].direction;
				set_sprite_data(i * NB_ENTITY_TILES, 16, &gfx_luvui[entities.array[i].direction * 256]);
				goto reload_frame;
			} else if (entities.array[i].spr_frame && (entities.array[i].spr_frame != entities.array[i].prev_frame)) {
				reload_frame:
				entities.array[i].prev_frame = entities.array[i].spr_frame;
			}
			const char *metasprite = &entities.array[i].data->metasprites[entities.array[i].spr_frame];
			if (anim_timer & 0b10000)
				metasprite += 4;

			shadow_OAM[oam_index].y = 16 + entities.array[i].y_spr;
			shadow_OAM[oam_index].x = 8 + entities.array[i].x_spr;
			shadow_OAM[oam_index].tile = metasprite[0] + i * NB_ENTITY_TILES;
			shadow_OAM[oam_index].prop = metasprite[1];
			oam_index++;
			shadow_OAM[oam_index].y = 16 + entities.array[i].y_spr;
			shadow_OAM[oam_index].x = 16 + entities.array[i].x_spr;
			shadow_OAM[oam_index].tile = metasprite[2] + i * NB_ENTITY_TILES;
			shadow_OAM[oam_index].prop = metasprite[3];
			oam_index++;
		}
	}
	anim_timer++;
}