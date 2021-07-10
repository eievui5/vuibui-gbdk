#pragma bank 255

#include <gb/cgb.h>
#include <gb/gb.h>
#include <rand.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "include/dir.h"
#include "include/entity.h"
#include "include/hud.h"
#include "include/int.h"
#include "include/map.h"
#include "include/move.h"
#include "include/rendering.h"
#include "include/vec.h"

#include "moves/scratch.h"
#include "moves/lunge.h"

#define DETECTION_RANGE 12

entity_array entities;
// The index of an ally which has been forced to move by the player. This ally
// loses their turn
u8 ignore_ally = 0;
u8 move_speed = 1;

// Reload an entity's graphics using its frame
void reload_entity_graphics(entity *self, u8 i) NONBANKED
{
	u8 temp_bank = _current_bank;
	SWITCH_ROM_MBC1(self->bank);

	self->prev_dir = self->direction;
	self->prev_frame = self->spr_frame;
	vmemcpy(
		(void *)(0x8000 + i * (16 * NB_ENTITY_TILES)),
		16 * NB_SPECIAL_TILES, &self->data->graphics[
			self->direction * 16 * NB_UNIQUE_TILES
		]
	);
	vmemcpy(
		(void *)(0x8000 + 16 * NB_SPECIAL_TILES + i * (16 * NB_ENTITY_TILES)),
		16 * NB_SPECIAL_TILES, &self->data->graphics[
			self->direction * 16 * NB_UNIQUE_TILES + 64 * self->spr_frame
		]
	);

	SWITCH_ROM_MBC1(temp_bank);
}

/**
 * Renders two 8x16 sprites at each entity's position.
*/
void render_entities() BANKED
{
	static u8 anim_timer = 0;
	entity *self = entities.array;
	for (u8 i = 0; i < NB_ENTITIES; i++, self++) {
		if (self->data) {
			if (self->spr_frame == HIDE_FRAME)
				continue;
			if (
				(16 + self->x_spr - camera.x & 0xFF) > win_pos.x &&
				(16 + self->y_spr - camera.y & 0xFF) > win_pos.y
			)
				continue;
			if (!(
				self->x_spr + 16 >= camera.x &&
				self->x_spr <= camera.x + 160 &&
				self->y_spr + 16 >= camera.y &&
				self->y_spr <= camera.y + 144
			))
				continue;

			// Update the entity's graphics if needed.
			if (
				self->direction != self->prev_dir ||
				self->spr_frame != self->prev_frame
			)
				reload_entity_graphics(self, i);
			const char *metasprite = self->data->metasprites;
			if (self->spr_frame > IDLE_FRAME)
				metasprite += 8;
			if (self->spr_frame <= WALK_FRAME && anim_timer & 0b10000)
				metasprite += 4;

			shadow_OAM[oam_index].y = 16 + self->y_spr - camera.y;
			shadow_OAM[oam_index].x = 8 + self->x_spr - camera.x;
			shadow_OAM[oam_index].tile = metasprite[0] + i * NB_ENTITY_TILES;
			shadow_OAM[oam_index].prop = i | metasprite[1];
			oam_index++;
			shadow_OAM[oam_index].y = 16 + self->y_spr - camera.y;
			shadow_OAM[oam_index].x = 16 + self->x_spr - camera.x;
			shadow_OAM[oam_index].tile = metasprite[2] + i * NB_ENTITY_TILES;
			shadow_OAM[oam_index].prop = i | metasprite[3];
			oam_index++;
		}
	}
	anim_timer += move_speed;
	clean_oam();
}

// Rendering function to move entities sprites towards their grid positions.
void move_entities() NONBANKED
{
	while(1) {
		u8 progress = 0;
		for (u8 i = 0; i < NB_ENTITIES; i++) {
			if (entities.array[i].data) {
				entities.array[i].spr_frame = WALK_FRAME;
				if (entities.array[i].x_pos * 16 != entities.array[i].x_spr) {
					if (entities.array[i].x_pos * 16 > entities.array[i].x_spr)
						entities.array[i].x_spr += move_speed;
					else
						entities.array[i].x_spr -= move_speed;
				} else if (entities.array[i].y_pos * 16 != entities.array[i].y_spr) {
					if (entities.array[i].y_pos * 16 > entities.array[i].y_spr)
						entities.array[i].y_spr += move_speed;
					else
						entities.array[i].y_spr -= move_speed;
				} else {
					entities.array[i].spr_frame = IDLE_FRAME;
					progress++;
				}
			} else
				progress++;
		}

		update_camera(
			entities.player.x_spr - 68,
			entities.player.y_spr - 64 + VCAM_OFF
		);

		if (progress == NB_ENTITIES) {
			wait_vbl_done();
			return;
		}

		render_entities();
		wait_vbl_done();
	}
}

/**
 * Plays the attack animation for an entity.
 * 
 * @param i	Entity's index.
*/
void attack_animation(entity *self) BANKED
{
	u8 j = 0;
	// Delay
	for (; j < 8; j++) {
		render_entities();
		wait_vbl_done();
	}

	// Windup
	uvec16 init_spr = {self->x_spr, self->y_spr};
	switch (self->direction) {
	case DIR_UP:
		self->y_spr += 1;
		break;
	case DIR_DOWN:
		self->y_spr -= 1;
		break;
	case DIR_LEFT:
		self->x_spr += 1;
		break;
	case DIR_RIGHT:
		self->x_spr -= 1;
		break;
	}
	for (j = 0; j < 3; j++) {
		render_entities();
		wait_vbl_done();
	}
	self->x_spr = init_spr.x;
	self->y_spr = init_spr.y;

	// Lunge
	self->spr_frame = HURT_FRAME;
	for (j = 0; j < 8; j++) {
		render_entities();
		wait_vbl_done();
	}
	self->spr_frame = ATTACK_FRAME;
	for (j = 0; j < 8; j++) {
		render_entities();
		wait_vbl_done();
	}
	self->spr_frame = IDLE_FRAME;
}

void hurt_animation(entity *self) BANKED
{
	u8 i = 0;

	self->spr_frame = HURT_FRAME;
	uvec16 init_spr = {self->x_spr, self->y_spr};
	for (; i < 3; i++) {
		switch (self->direction) {
		case DIR_UP:
			self->y_spr += 1;
			break;
		case DIR_DOWN:
			self->y_spr -= 1;
			break;
		case DIR_LEFT:
			self->x_spr += 1;
			break;
		case DIR_RIGHT:
			self->x_spr -= 1;
			break;
		}
		render_entities();
		wait_vbl_done();
		render_entities();
		wait_vbl_done();
	}
	u8 swap_dir = self->direction;
	for (i = 0; i < 3; i++) {
		switch (swap_dir) {
		case DIR_UP:
			self->y_spr += 1;
			break;
		case DIR_DOWN:
			self->y_spr -= 1;
			break;
		case DIR_LEFT:
			self->x_spr += 1;
			break;
		case DIR_RIGHT:
			self->x_spr -= 1;
			break;
		}
		swap_dir = FLIP(swap_dir);
		render_entities();
		wait_vbl_done();
		render_entities();
		wait_vbl_done();
		render_entities();
		wait_vbl_done();
		render_entities();
		wait_vbl_done();
	}

	self->x_spr = init_spr.x;
	self->y_spr = init_spr.y;
	self->spr_frame = IDLE_FRAME;
}

void defeat_animation(entity *self) BANKED
{
	hurt_animation(self);
	switch (self->direction) {
	case DIR_UP:
		self->y_spr += 3;
		break;
	case DIR_DOWN:
		self->y_spr -= 3;
		break;
	case DIR_LEFT:
		self->x_spr += 3;
		break;
	case DIR_RIGHT:
		self->x_spr -= 3;
		break;
	}
	for (u8 i = 0; i < 10; i++) {
		self->spr_frame = HIDE_FRAME;
		render_entities();
		wait_vbl_done();
		render_entities();
		wait_vbl_done();
		self->spr_frame = HURT_FRAME;
		render_entities();
		wait_vbl_done();
		render_entities();
		wait_vbl_done();
	}
}

/**
 * Create a new entity inside the entity array. Requires an index to load the
 * sprites into VRAM.
 * 
 * @param data		Pointer to the new entity's constant data.
 * @param bank		Bank of the entity's constant data.
 * @param i		Index of the entity.
 * @param x		Location to place the entity at.
 * @param y		
 * @param health	Temporary - Set health and max health.
*/
entity *new_entity(entity_data *data, u8 bank, u8 i, u8 x, u8 y, u16 health) NONBANKED
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
	self->health = health;
	self->max_health = health;
	self->moves[0].data = &scratch_move;
	self->moves[0].bank = BANK(scratch);
	self->moves[1].data = &lunge_move;
	self->moves[1].bank = BANK(lunge);
	strcpy(self->name, data->name);

	u8 temp_bank = _current_bank;
	SWITCH_ROM_MBC1(bank);
	vmemcpy(
		(void *)(0x8000 + i * (16 * NB_ENTITY_TILES)),
		NB_ENTITY_TILES * 16,
		entities.array[i].data->graphics
	);
	if (_cpu == CGB_TYPE)
		set_sprite_palette(i, 1, entities.array[i].data->colors);
	SWITCH_ROM_MBC1(temp_bank);
	return self;
}

/**
 * Attempt to spawn an entity, allocating space for it in the entity array.
 * 
 * @param data		Pointer to the new entity's constant data.
 * @param bank		Bank of the entity's constant data.
 * @param i		Index of the entity.
 * @param x		Location to place the entity at.
 * @param y		
*/
entity *spawn_enemy(entity_data *data, u8 bank) BANKED
{
	entity *self = entities.enemies;
	for (u8 i = 3; i < NB_ENTITIES; i++, self++)
		if (!self->data) {
			u8 x;
			u8 y;
			while(1) {
				x = rand() & 0b111111;
				y = rand() & 0b111111;
				if (!check_collision(x, y))
					break;
			}
			return new_entity(data, bank, i, x, y, 4);
		}
	return NULL;
}

/**
 * Offset a vector (a pointer of course... thanks SDCC...) based on a direction
 * value.
 * 
 * @param vec	Pointer to the target vector.
 * @param dir	The direction to use as an offset.
*/
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
 * Return the direction of a given vector. Prefers X over Y, and returns DIR_UP
 * for 0, 0.
 * 
 * @param x	X direction.
 * @param y	Y direction.
 * 
 * @return	Resulting direction value.
*/
u8 get_direction(i8 x, i8 y)
{
	if (abs(x) > abs(y)) {
		if (x > 0)
			return DIR_RIGHT;
		else
			return DIR_LEFT;
	}
	if (y > 0)
		return DIR_DOWN;
	return DIR_UP;
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

/**
 * A variation of try_step() to alow the player to swap positions with their
 * allies.
 * 
 * @returns	Whether or not movement succeeded.
*/
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
			goto skip_enemies;
		}
	}
	for (u8 i = 0; i < NB_ENEMIES; i++)
		if (entities.enemies[i].x_pos == target_x && entities.enemies[i].y_pos == target_y)
			return false;
	skip_enemies:
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
entity *check_entity_at(u8 x, u8 y) BANKED
{
	for (u8 i = 0; i < NB_ENTITIES; i++) {
		if (!entities.array[i].data)
			continue;
		if (entities.array[i].x_pos == x && entities.array[i].y_pos == y)
			return &entities.array[i];
	}
	return NULL;
}

/**
 * Checks for collision or an entity at a given location.
 * 
 * @returns	True if the location is valid.
*/
bool check_collision(u8 x, u8 y) BANKED
{
	if (get_collision(x, y))
		return true;
	return (bool)check_entity_at(x, y);
}

/**
 * Attempt to move toward a given location.
 * 
 * @param self		A pointer to the entity to move.
 * @param target_x	Target location.
 * @param target_y
*/
void pathfind(entity *self, u8 target_x, u8 target_y) BANKED
{
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

/**
 * Attempt to chaser the allies and attack them if within range.
 * 
 * @param i	Index of the pursuing entity.
 * @param start Starting index of the array to pursue (usually 0 or 3)
 * @param stop	Stopping index of the array to pursue (usually 3 or 8)
*/
void pursue(entity *self, u8 start, u8 stop) BANKED
{
	entity *ally = &entities.array[start];
	i8 closest = -1;
	u16 dist = 65535;
	for (u8 i = start; i < stop; i++, ally++) {
		u16 cur_dist = (
			abs(self->x_pos - ally->x_pos) +
			abs(self->y_pos - ally->y_pos)
		);
		if (cur_dist < dist) {
			dist = cur_dist;
			closest = i;
		}
	}
	ally = &entities.array[closest];
	if (dist == 1) {
		self->direction = get_direction(
			ally->x_pos - self->x_pos, ally->y_pos - self->y_pos
		);
		use_melee_move(self, &self->moves[0]);
	} else
		pathfind(self, ally->x_pos, ally->y_pos);
}