#pragma bank 255

#include <gb/gb.h>
#include <stdbool.h>
#include "include/dir.h"
#include "include/entity.h"
#include "include/int.h"
#include "include/map.h"
#include "include/rendering.h"

#define mspd 1

entity_array entities;

/**
 * Renders two 8x16 sprites at each entity's position. The tile is chosen based
 * on the entity's index, allowing each entity to stream its own graphics.
*/
void render_entities() NONBANKED
{
	static u8 anim_timer = 0;

	for (u8 i = 0; i < NB_ENTITIES; i++) {
		if (entities.array[i].data) {

			if (!(
				entities.array[i].x_spr + 16 >= camera_x &&
				entities.array[i].x_spr <= camera_x + 160 &&
				entities.array[i].y_spr + 16 >= camera_y &&
				entities.array[i].y_spr <= camera_y + 144
			)) continue;

			// Update the entity's graphics if needed.
			if (entities.array[i].direction != entities.array[i].prev_dir) {
				entities.array[i].prev_dir = entities.array[i].direction;
				SWITCH_ROM_MBC1(entities.array[i].bank);
				set_sprite_data(
					i * NB_ENTITY_TILES, NB_ENTITY_TILES,
					&entities.array[i].data->graphics[
						entities.array[i].direction * 256
					]
				);
			}
			const char *metasprite = &entities.array[i].data->metasprites[entities.array[i].spr_frame];
			if (anim_timer & 0b10000)
				metasprite += 4;

			shadow_OAM[oam_index].y = 16 + entities.array[i].y_spr - camera_y;
			shadow_OAM[oam_index].x = 8 + entities.array[i].x_spr - camera_x;
			shadow_OAM[oam_index].tile = metasprite[0] + i * NB_ENTITY_TILES;
			shadow_OAM[oam_index].prop = metasprite[1];
			oam_index++;
			shadow_OAM[oam_index].y = 16 + entities.array[i].y_spr - camera_y;
			shadow_OAM[oam_index].x = 16 + entities.array[i].x_spr - camera_x;
			shadow_OAM[oam_index].tile = metasprite[2] + i * NB_ENTITY_TILES;
			shadow_OAM[oam_index].prop = metasprite[3];
			oam_index++;
		}
	}
	anim_timer++;
	clean_oam();
}

void move_entities() NONBANKED
{
	while(1) {
		u8 progress = 0;
		for (u8 i = 0; i < NB_ENTITIES; i++) {
			if (entities.array[i].data) {
				entities.array[i].spr_frame = 8;
				if ((entities.array[i].x_pos & 0xFF) * 16 != entities.array[i].x_spr) {
					if ((entities.array[i].x_pos & 0xFF) * 16 > entities.array[i].x_spr)
						entities.array[i].x_spr += mspd;
					else
						entities.array[i].x_spr -= mspd;
				} else if ((entities.array[i].y_pos & 0xFF) * 16 != entities.array[i].y_spr) {
					if ((entities.array[i].y_pos & 0xFF) * 16 > entities.array[i].y_spr)
						entities.array[i].y_spr += mspd;
					else
						entities.array[i].y_spr -= mspd;
				} else {
					entities.array[i].spr_frame = 0;
					progress++;
				}
			} else
				progress++;
		}

		update_camera(
			entities.player.x_spr - 68,
			entities.player.y_spr - 64 + VCAM_OFF
		);

		if (progress == NB_ENTITIES)
			return;

		render_entities();
		wait_vbl_done();
	}
}

bool try_step(u8 i, u8 dir) BANKED
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

/**
 * Check for an entity at a given grid position.
 * 
 * @param ignore	Index of the calling entity, used to ignore self.
 * @param x		X position to check.
 * @param y		Y position to check.
 * 
 * @returns		The detected entity. NULL if no entity is found.
*/
entity *check_entity_collision(u8 ignore, u8 x, u8 y) BANKED
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

bool check_collision(u8 ignore, u8 x, u8 y) BANKED
{
	if (get_collision(x, y))
		return true;
	else if (check_entity_collision(ignore, x, y))
		return true;
	return false;
}