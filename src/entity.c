#pragma bank 255

#include <gb/gb.h>
#include <rand.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "include/dir.h"
#include "include/entity.h"
#include "include/int.h"
#include "include/map.h"
#include "include/rendering.h"
#include "include/vec.h"

#define mspd 1
#define DETECTION_RANGE 12

entity_array entities;
// The index of an ally which has been forced to move by the player. This ally
// loses their turn
u8 ignore_ally = 0;

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
			shadow_OAM[oam_index].prop = i | metasprite[1];
			oam_index++;
			shadow_OAM[oam_index].y = 16 + entities.array[i].y_spr - camera.y;
			shadow_OAM[oam_index].x = 16 + entities.array[i].x_spr - camera.x;
			shadow_OAM[oam_index].tile = metasprite[2] + i * NB_ENTITY_TILES;
			shadow_OAM[oam_index].prop = i | metasprite[3];
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

void new_entity(entity_data *data, u8 bank, u8 i, u8 x, u8 y) BANKED
{
	entity *self = &entities.array[i];
	memset(self, 0, sizeof(entity));
	if (entities.player.data) {
		if (abs(entities.player.x_pos - x) > abs(entities.player.y_pos - y)) {
			if (entities.player.x_pos - x > 0) {
				self->direction = DIR_RIGHT;
			} else {
				self->direction = DIR_LEFT;
			}
		} else {
			if (entities.player.y_pos - y > 0) {
				self->direction = DIR_DOWN;
			}
			else {
				self->direction = DIR_UP;
			}
		}
	}
	self->data = data;
	self->bank = bank;
	self->x_pos = x;
	self->x_spr = x * 16;
	self->y_pos = y;
	self->y_spr = y * 16;
	self->path_dir = -1;
}

/**
 * Spawns an entity at a given location, allocating space for it in the entity
 * array.
*/
bool spawn_enemy(entity_data *data, u8 bank, u8 x, u8 y) BANKED
{
	for (u8 i = 0; i < NB_ENEMIES; i++)
		if (!entities.enemies[i].data) {
			new_entity(data, bank, i, x, y);
			return true;
		}
	return false;
}

void move_direction(vec8 *vec, u8 dir) BANKED
{
	switch (dir) {
	case DIR_UP:
		vec->y--;
		break;
	case DIR_RIGHT:
		vec->x++;
		break;
	case DIR_DOWN:
		vec->y++;
		break;
	case DIR_LEFT:
		vec->x--;
		break;
	}
}

/**
 * Attempt to move in a given direction.
 * 
 * @param self	The entity to move.
 * @param dir	The direction to attempt movement in.
 * 
 * @returns	Whether or not movement succeeded.
*/
bool try_step(entity *self, u8 dir) BANKED
{
	self->direction = dir;
	vec8 target = {self->x_pos, self->y_pos};
	move_direction(&target, dir);
	if (!check_collision(target.x, target.y)) {
		self->x_pos = target.x;
		self->y_pos = target.y;
		return true;
	}
	return false;
}

bool player_try_step() BANKED
{
	u8 target_x = entities.player.x_pos;
	u8 target_y = entities.player.y_pos;
	switch (entities.player.direction) {
	case DIR_UP:
		target_y--;
		break;
	case DIR_RIGHT:
		target_x++;
		break;
	case DIR_DOWN:
		target_y++;
		break;
	case DIR_LEFT:
		target_x--;
		break;
	}
	if (get_collision(target_x, target_y))
		return false;
	for (u8 i = 1; i < NB_ALLIES; i++) {
		if (!entities.allies[i].data)
			continue;
		if (entities.allies[i].x_pos == target_x && entities.allies[i].y_pos == target_y) {
			entities.allies[i].x_pos = entities.player.x_pos;
			entities.allies[i].y_pos = entities.player.y_pos;
			entities.allies[i].direction = FLIP(entities.player.direction);
			ignore_ally = i;
			goto move;
		}
	}
	for (u8 i = 0; i < NB_ENEMIES; i++)
		if (entities.array[i].x_pos == target_x && entities.array[i].y_pos == target_y)
			return false;
	move:
	entities.player.x_pos = target_x;
	entities.player.y_pos = target_y;
	return true;
}

/**
 * Check for an entity at a given grid position.
 * 
 * @param x		X position to check.
 * @param y		Y position to check.
 * 
 * @returns		The detected entity. NULL if no entity is found.
*/
entity *check_entity_collision(u8 x, u8 y) BANKED
{
	for (u8 i = 0; i < NB_ENTITIES; i++) {
		if (!entities.array[i].data)
			continue;
		if (entities.array[i].x_pos == x && entities.array[i].y_pos == y)
			return &entities.array[i];
	}
	return NULL;
}

bool check_collision(u8 x, u8 y) BANKED
{
	if (get_collision(x, y))
		return true;
	else if (check_entity_collision(x, y))
		return true;
	return false;
}

void pathfind(entity *self, u8 target_x, u8 target_y) BANKED {
	i8 dist_x = target_x - self->x_pos;
	i8 dist_y = target_y - self->y_pos;
	i8 dir = -1;
	i8 dir2 = -1; // Secondary choice if choice one fails.
	if (abs(dist_x) > abs(dist_y)) {
		if (dist_x > 0)
			dir = DIR_RIGHT;
		else
			dir = DIR_LEFT;
		if (dist_y) {
			if (dist_y > 0)
				dir2 = DIR_DOWN;
			else
				dir2 = DIR_UP;
		}
	} else if (dist_y) {
		if (dist_y > 0)
			dir = DIR_DOWN;
		else
			dir = DIR_UP;
		if (dist_x) {
			if (dist_x > 0)
				dir2 = DIR_RIGHT;
			else
				dir2 = DIR_LEFT;
		}
	}
	if (dir != -1)
		if (!try_step(self, dir))
			if (dir2 != -1)
				try_step(self, dir2);
	else if (dir2 != -1)
		try_step(self, dir2);
}

void pursue(entity *self) BANKED
{
	entity *ally = entities.allies;
	//for (u8 i = 0; i < NB_ALLIES; i++, ally++) {
		pathfind(self, ally->x_pos, ally->y_pos);
	//	return;
	//}
}

// Handle ally and enemy AI
void do_turn() BANKED
{
	for (u8 i = 1; i < NB_ALLIES; i++) {
		if (i == ignore_ally) {
			ignore_ally = 0;
			continue;
		}
		if (entities.allies[i].data) {
			pathfind(
				&entities.allies[i],
				entities.player.x_pos,
				entities.player.y_pos
			);
		}
	}
	for (u8 i = 0; i < NB_ENEMIES; i++) {
		if (entities.enemies[i].data)
			pursue(&entities.enemies[i]);
	}
	move_entities();
}