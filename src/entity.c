#pragma bank 255

#include <gb/gb.h>
#include <rand.h>
#include <stdbool.h>
#include <stdlib.h>

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
				entities.array[i].x_spr + 16 >= camera.x &&
				entities.array[i].x_spr <= camera.x + 160 &&
				entities.array[i].y_spr + 16 >= camera.y &&
				entities.array[i].y_spr <= camera.y + 144
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

			shadow_OAM[oam_index].y = 16 + entities.array[i].y_spr - camera.y;
			shadow_OAM[oam_index].x = 8 + entities.array[i].x_spr - camera.x;
			shadow_OAM[oam_index].tile = metasprite[0] + i * NB_ENTITY_TILES;
			shadow_OAM[oam_index].prop = metasprite[1];
			oam_index++;
			shadow_OAM[oam_index].y = 16 + entities.array[i].y_spr - camera.y;
			shadow_OAM[oam_index].x = 16 + entities.array[i].x_spr - camera.x;
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

bool try_step(entity *self, u8 i, u8 dir) BANKED
{
	self->direction = dir;
	u16 target_x = self->x_pos;
	u16 target_y = self->y_pos;
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
		self->x_pos = target_x;
		self->y_pos = target_y;
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

void pathfind(entity *self, u8 i, u8 target_x, u8 target_y) {
	i8 dist_x = target_x - self->x_pos;
	i8 dist_y = target_y - self->y_pos;
	i8 dir = -1;
	i8 dir2 = -1; // Secondary choice if choice one fails.
	if (abs(dist_x) > abs(dist_y)) {
		if (dist_x > 0)
			dir = DIR_RIGHT;
		else
			dir = DIR_LEFT;
		if (dist_y)
			if (dist_y > 0)
				dir2 = DIR_DOWN;
			else
				dir2 = DIR_UP;
	} else if (dist_y) {
		if (dist_y > 0)
			dir = DIR_DOWN;
		else
			dir = DIR_UP;
		if (dist_x)
			if (dist_x > 0)
				dir2 = DIR_RIGHT;
			else
				dir2 = DIR_LEFT;
	}
	if (dir != -1)
		if (!try_step(self, i, dir))
			if (dir2 != -1)
				try_step(self, i, dir2);
	else if (dir2 != -1)
		try_step(self, i, dir2);

}

// Handle ally and enemy AI
void do_turn()
{
	for (u8 i = 1; i < NB_ALLIES; i++) {
		if (entities.allies[i].data) {
			pathfind(
				&entities.allies[i], i,
				entities.player.x_pos,
				entities.player.y_pos
			);
		}
	}
	for (u8 i = 0; i < NB_ENEMIES; i++) {
		if (entities.enemies[i].data)
			try_step(&entities.enemies[i], i, rand() & 0b11);
	}
	move_entities();
}