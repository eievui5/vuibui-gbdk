#pragma bank 255

#include <gb/cgb.h>
#include <gb/gb.h>
#include <rand.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "include/dir.h"
#include "include/entity.h"
#include "include/hud.h"
#include "include/map.h"
#include "include/move.h"
#include "include/rendering.h"
#include "include/vec.h"

entity entities[NB_ENTITIES];
// The index of an ally which has been forced to move by the player. This ally
// loses their turn
uint8_t ignore_ally = 0;
uint8_t move_speed = 1;

const char metasprite_table[] = {
	// Idle.
	0, 0,
	2, 0,
	// Idle Flip.
	4, 0,
	6, 0,
	// Step.
	8, 0,
	10, 0,
	// Step Flip.
	12, 0,
	14, 0,
};

// Reload an entity's graphics using its frame
void reload_entity_graphics(uint8_t i) NONBANKED
{
	entity *self = &entities[i];
	uint8_t temp_bank = _current_bank;
	SWITCH_ROM_MBC1(self->bank);

	self->prev_dir = self->direction;
	self->prev_frame = self->spr_frame;
	vmemcpy((void *)(0x8000 + i * (16 * NB_ENTITY_TILES)),
		16 * NB_SPECIAL_TILES, &self->data->graphics[
			self->direction * 16 * NB_UNIQUE_TILES]);
	vmemcpy((void *)(0x8000 + 16 * NB_SPECIAL_TILES + i * (16 * NB_ENTITY_TILES)),
		16 * NB_SPECIAL_TILES, &self->data->graphics[
			self->direction * 16 * NB_UNIQUE_TILES + 64 * self->spr_frame]);
	if (_cpu == CGB_TYPE)
		set_sprite_palette(i, 1, entities[i].data->colors);

	SWITCH_ROM_MBC1(temp_bank);
}

// Draws an entity with a static frame and direction.
void draw_static_entity(entity *src, uint8_t dir, uint8_t frame, uint8_t* dest,
			int8_t pal) NONBANKED
{
	uint8_t temp_bank = _current_bank;
	SWITCH_ROM_MBC1(src->bank);
	vmemcpy(dest, 64, &src->data->graphics[
		dir * NB_UNIQUE_TILES  * 16 + 64 * frame]);
	if (_cpu == CGB_TYPE && pal != -1)
		set_sprite_palette(pal, 1, src->data->colors);
	SWITCH_ROM_MBC1(temp_bank);
}

/**
 * Renders two 8x16 sprites at each entity's position.
*/
void render_entities() BANKED
{
	static uint8_t anim_timer = 0;
	entity *self = entities;
	for (uint8_t i = 0; i < NB_ENTITIES; i++, self++) {
		if (self->data) {
			if (self->spr_frame == HIDE_FRAME)
				continue;
			if ((16 + self->x_spr - camera.x & 0xFF) > win_pos.x &&
			    (16 + self->y_spr - camera.y & 0xFF) > win_pos.y)
				continue;
			if (!(self->x_spr + 16 >= camera.x &&
			    self->x_spr <= camera.x + 160 &&
			    self->y_spr + 16 >= camera.y &&
			    self->y_spr <= camera.y + 144))
				continue;

			// Update the entity's graphics if needed.
			if (self->direction != self->prev_dir ||
			    self->spr_frame != self->prev_frame)
				reload_entity_graphics(i);

			uint8_t sprite_offset = 0;
			if (self->spr_frame > IDLE_FRAME)
				sprite_offset += 8;
			if (self->spr_frame <= WALK_FRAME && anim_timer & 0b10000)
				sprite_offset += 4;
			const char *metasprite = &metasprite_table[sprite_offset];

			shadow_OAM[oam_index].y = 16 + self->y_spr - camera.y;
			shadow_OAM[oam_index].x = 8 + self->x_spr - camera.x;
			shadow_OAM[oam_index].tile = *metasprite++ + i * NB_ENTITY_TILES;
			shadow_OAM[oam_index].prop = i | *metasprite++;
			oam_index++;
			shadow_OAM[oam_index].y = 16 + self->y_spr - camera.y;
			shadow_OAM[oam_index].x = 16 + self->x_spr - camera.x;
			shadow_OAM[oam_index].tile = *metasprite++ + i * NB_ENTITY_TILES;
			shadow_OAM[oam_index].prop = i | *metasprite;
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
		uint8_t progress = 0;
		for (uint8_t i = 0; i < NB_ENTITIES; i++) {
			if (entities[i].data) {
				entities[i].spr_frame = WALK_FRAME;
				if (entities[i].x_pos * 16 != entities[i].x_spr) {
					if (entities[i].x_pos * 16 > entities[i].x_spr)
						entities[i].x_spr += move_speed;
					else
						entities[i].x_spr -= move_speed;
				} else if (entities[i].y_pos * 16 != entities[i].y_spr) {
					if (entities[i].y_pos * 16 > entities[i].y_spr)
						entities[i].y_spr += move_speed;
					else
						entities[i].y_spr -= move_speed;
				} else {
					entities[i].spr_frame = IDLE_FRAME;
					progress++;
				}
			} else
				progress++;
		}

		update_camera(
			PLAYER.x_spr - 68,
			PLAYER.y_spr - 64 + VCAM_OFF
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
	uint8_t j = 0;
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
	uint8_t i = 0;

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
	uint8_t swap_dir = self->direction;
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
	for (uint8_t i = 0; i < 10; i++) {
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

uint16_t get_xp_reward(entity *self) NONBANKED
{
	uint8_t temp_bank = _current_bank;
	SWITCH_ROM_MBC1(self->bank);
	uint16_t reward = self->data->base_xp + (self->data->base_xp / 2) * self->level;
	SWITCH_ROM_MBC1(temp_bank);
	return reward;
}

uint16_t get_max_health(entity *self) NONBANKED
{
	uint8_t temp_bank = _current_bank;
	SWITCH_ROM_MBC1(self->bank);
	uint16_t max_health = self->data->base_health + (self->data->base_health / 8) * self->level;
	SWITCH_ROM_MBC1(temp_bank);
	return max_health;
}

uint16_t get_max_fatigue(entity *self) NONBANKED
{
	uint8_t temp_bank = _current_bank;
	SWITCH_ROM_MBC1(self->bank);
	uint16_t max_fatigue = (self->data->base_fatigue + ((self->data->base_fatigue * self->level) / 256)) * 16;
	SWITCH_ROM_MBC1(temp_bank);
	return max_fatigue;
}

uint16_t get_attack_bonus(entity *self) NONBANKED
{
	uint8_t temp_bank = _current_bank;
	SWITCH_ROM_MBC1(self->bank);
	uint16_t attack_bonus = (2 * self->data->base_attack + self->data->base_attack * self->level) / 16;
	SWITCH_ROM_MBC1(temp_bank);
	return attack_bonus;
}

uint16_t get_defense_bonus(entity *self) NONBANKED
{
	uint8_t temp_bank = _current_bank;
	SWITCH_ROM_MBC1(self->bank);
	uint16_t defense_bonus = (self->data->base_defense + (self->data->base_defense * self->level) / 8) / 8;
	SWITCH_ROM_MBC1(temp_bank);
	return defense_bonus;
}

// Banked portion of `new_entity`
void generate_moves(entity *self) NONBANKED
{
	uint8_t temp_bank = _current_bank;
	SWITCH_ROM_MBC1(self->bank);
	const entity_data *self_data = self->data;

	// Grab the last 4 moves the entity learned, unless they have not yet
	// learned 4.
	int8_t cur_move_num = 0;
	const struct leveled_move *cur_move = self_data->level_moves;
	while ((cur_move + 1)->level < self->level) {
		cur_move++;
		cur_move_num++;
	}

	uint8_t i;
	move *self_move = self->moves;
	for (i = 0; i < 4 && cur_move_num >= 0; i++) {
		self_move->bank = cur_move->bank;
		self_move->data = cur_move->data;
		self_move++;
		cur_move--;
		cur_move_num--;
	}

	// Also copy the name from ROM while you're at it - this is only for
	// `new_entity` after all.
	strcpy(self->name, self_data->name);

	SWITCH_ROM_MBC1(temp_bank);
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
entity *new_entity(const entity_data *data, uint8_t bank, uint8_t i, uint8_t x, 
		   uint8_t y, uint8_t level) BANKED
{
	entity *self = &entities[i];
	memset(self, 0, sizeof(entity));
	if (PLAYER.data) {
		if (abs(PLAYER.x_pos - x) > abs(PLAYER.y_pos - y)) {
			if (PLAYER.x_pos - x > 0)
				self->direction = DIR_RIGHT;
			else
				self->direction = DIR_LEFT;
		} else {
			if (PLAYER.y_pos - y > 0)
				self->direction = DIR_DOWN;
			else
				self->direction = DIR_UP;
		}
	}
	self->data = data;
	self->bank = bank;
	self->x_pos = x;
	self->x_spr = x * 16;
	self->y_pos = y;
	self->y_spr = y * 16;
	self->path_dir = -1;
	self->level = level;
	self->health = get_max_health(self);
	self->fatigue = get_max_fatigue(self);
	generate_moves(self);
	reload_entity_graphics(i);
	if (!i)
		self->team = PLAYER_TEAM;
	else if (i < 3)
		self->team = ALLY_TEAM;
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
entity *spawn_enemy(const entity_data *data, uint8_t bank, uint8_t level) BANKED
{
	entity *self = &entities[3];
	for (uint8_t i = 3; i < NB_ENTITIES; i++, self++) {
		if (!self->data) {
			uint8_t x;
			uint8_t y;
			while(1) {
				x = rand() & 0b111111;
				y = rand() & 0b111111;
				if (!check_collision(x, y) && 
				    !(x * 16 + 16 >= camera.x &&
				    x * 16 <= camera.x + 160 &&
				    y * 16 + 16 >= camera.y &&
				    y * 16 <= camera.y + 144))
					break;
			}
			return new_entity(data, bank, i, x, y, level);
		}
	}
	return NULL;
}

/**
 * Attempt to move in a given direction.
 *
 * @param self	The entity to move.
 * @param dir	The direction to attempt movement in.
 *
 * @returns	Whether or not movement succeeded.
*/
bool try_step(entity *self, uint8_t dir) BANKED
{
	self->direction = dir;
	vec8 target = {self->x_pos, self->y_pos};
	move_direction(&target, dir);
	if (!check_collision(target.x, target.y)) {
		if (self->fatigue) {
			self->fatigue -= WALK_COST;
		} else {
			self->health--;
		}
		self->x_pos = target.x;
		self->y_pos = target.y;
		return true;
	}
	return false;
}

/**
 * A variation of try_step() to alow the allies to swap positions with each 
 * other.
 *
 * @returns	Whether or not movement succeeded.
*/
bool ally_try_step(entity *self) BANKED
{
	uint8_t target_x = self->x_pos;
	uint8_t target_y = self->y_pos;
	switch (self->direction) {
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
	if (get_collision(target_x, target_y) == WALL_COLL)
		return false;
	for (uint8_t i = 1; i < NB_ALLIES; i++) {
		if (!entities[i].data)
			continue;
		if (entities[i].x_pos == target_x && \
		    entities[i].y_pos == target_y) {
			entities[i].x_pos = self->x_pos;
			entities[i].y_pos = self->y_pos;
			entities[i].direction = FLIP(self->direction);
			ignore_ally = i;
			goto skip_enemies;
		}
	}
	for (uint8_t i = BEGIN_ENEMIES; i < NB_ENTITIES; i++)
		if (entities[i].x_pos == target_x &&
		    entities[i].y_pos == target_y)
			return false;
	skip_enemies:
	self->x_pos = target_x;
	self->y_pos = target_y;
	return true;
}

/**
 * Check for an entity at a given grid position.
 *
 * @param x	X position to check.
 * @param y	Y position to check.
 *		
 * @returns	The index of the detected entity. -1 if no entity is found.
*/
int8_t check_entity_at(uint8_t x, uint8_t y) BANKED
{
	for (uint8_t i = 0; i < NB_ENTITIES; i++) {
		if (!entities[i].data)
			continue;
		if (entities[i].x_pos == x && entities[i].y_pos == y)
			return i;
	}
	return -1;
}

/**
 * Checks for collision or an entity at a given location.
 *
 * @returns	True if the location is valid.
*/
bool check_collision(uint8_t x, uint8_t y) BANKED
{
	if (get_collision(x, y) == WALL_COLL)
		return true;
	return check_entity_at(x, y) != -1;
}

/**
 * Attempt to move toward a given location. Pathfinding will always attempt to
 * move as close to the target as possible, turning to move around objects if
 * needed.
 *
 * @param self		A pointer to the entity to move.
 * @param target_x	Target location.
 * @param target_y
*/
void pathfind(entity *self, uint8_t target_x, uint8_t target_y) BANKED
{
	int8_t dist_x = target_x - self->x_pos;
	int8_t dist_y = target_y - self->y_pos;
	int8_t dir;
	int8_t dir2; // Secondary choice if choice one fails.
	if (abs(dist_x) > abs(dist_y)) {
		if (dist_x > 0)
			dir = DIR_RIGHT;
		else
			dir = DIR_LEFT;
		if (dist_y > 0)
			dir2 = DIR_DOWN;
		else
			dir2 = DIR_UP;
	} else {
		if (dist_y > 0)
			dir = DIR_DOWN;
		else
			dir = DIR_UP;
		if (dist_x > 0)
			dir2 = DIR_RIGHT;
		else
			dir2 = DIR_LEFT;
	}
	// Try to move towards the enemy. If you cannot move directly, find a
	if (!try_step(self, dir)) {
		vec8 target_pos = {self->x_pos, self->y_pos};
		move_direction(&target_pos, dir2);
		move_direction(&target_pos, dir);
		if (get_collision(target_pos.x, target_pos.y) != WALL_COLL)
			if (try_step(self, dir2))
				return;
		try_step(self, FLIP(dir2));
	}
}

/**
 * Attempt to move toward a given location.
 *
 * @param self		A pointer to the entity to move.
 * @param target_x	Target location.
 * @param target_y
*/
void ally_pathfind(entity *self, uint8_t target_x, uint8_t target_y) BANKED
{
	int8_t dist_x = target_x - self->x_pos;
	int8_t dist_y = target_y - self->y_pos;
	if (abs(dist_x) + abs(dist_y) == 1)
		return;
	int8_t dir;
	int8_t dir2; // Secondary choice if choice one fails.
	if (abs(dist_x) > abs(dist_y)) {
		if (dist_x > 0)
			dir = DIR_RIGHT;
		else
			dir = DIR_LEFT;
		if (dist_y > 0)
			dir2 = DIR_DOWN;
		else
			dir2 = DIR_UP;
	} else {
		if (dist_y > 0)
			dir = DIR_DOWN;
		else
			dir = DIR_UP;
		if (dist_x > 0)
			dir2 = DIR_RIGHT;
		else
			dir2 = DIR_LEFT;
	}
	// Try to move towards the enemy. If you cannot move directly, find a
	self->direction = dir;
	if (!ally_try_step(self)) {
		vec8 target_pos = {self->x_pos, self->y_pos};
		move_direction(&target_pos, dir2);
		move_direction(&target_pos, dir);
		if (get_collision(target_pos.x, target_pos.y) != WALL_COLL) {
			self->direction = dir2;
			if (ally_try_step(self))
				return;
		}
		self->direction = FLIP(dir2);
		ally_try_step(self);
	}
}

bool detection_trace(uint8_t self_x, uint8_t self_y, uint8_t target_x, 
		      uint8_t target_y, uint8_t distance) BANKED
{
	vec8 self_pos = {self_x, self_y};
	for (uint8_t i = 0; i < distance; i++) {
		int8_t dist_x = target_x - self_pos.x;
		int8_t dist_y = target_y - self_pos.y;
		if (abs(dist_x) + abs(dist_y) == 1)
			return true;
		int8_t dir;
		int8_t dir2; // Secondary choice if choice one fails.
		if (abs(dist_x) > abs(dist_y)) {
			if (dist_x > 0)
				dir = DIR_RIGHT;
			else
				dir = DIR_LEFT;
			if (dist_y > 0)
				dir2 = DIR_DOWN;
			else
				dir2 = DIR_UP;
		} else {
			if (dist_y > 0)
				dir = DIR_DOWN;
			else
				dir = DIR_UP;
			if (dist_x > 0)
				dir2 = DIR_RIGHT;
			else
				dir2 = DIR_LEFT;
		}
		// Try to move towards the enemy. If you cannot move directly, find a
		vec8 target_pos = {self_pos.x, self_pos.y};
		move_direction(&target_pos, dir);
		if (get_collision(target_pos.x, target_pos.y) != WALL_COLL) {
			move_direction(&self_pos, dir);
			continue;
		}
		move_direction(&target_pos, dir2);
		if (get_collision(target_pos.x, target_pos.y) != WALL_COLL) {
			move_direction(&self_pos, dir2);
			continue;
		}
		move_direction(&target_pos, FLIP(dir2));
		move_direction(&target_pos, FLIP(dir2));
		if (get_collision(target_pos.x, target_pos.y) != WALL_COLL) {
			move_direction(&self_pos, FLIP(dir2));
			continue;
		}
		return false;
	}
	return false;
}

/**
 * Attempt to chase a group of entities and attack them if within range.
 *
 * @param i	Index of the pursuing entity.
 * @param start Starting index of the array to pursue (usually 0 or 3)
 * @param stop	Stopping index of the array to pursue (usually 3 or 8)
*/
void pursue(entity *self, uint8_t start, uint8_t stop) BANKED
{
	entity *target = &entities[start];
	int8_t closest = -1;
	uint16_t dist = 65535;
	for (uint8_t i = start; i < stop; i++, target++) {
		if (!target->data)
			continue;
		uint16_t cur_dist = abs(self->x_pos - target->x_pos) +
				    abs(self->y_pos - target->y_pos);
		if (cur_dist < dist) {
			dist = cur_dist;
			closest = i;
		}
	}
	if (closest == -1)
		return;
	target = &entities[closest];
	if (dist == 1) {
		self->direction = get_direction(target->x_pos - self->x_pos,
						target->y_pos - self->y_pos);
		use_melee_move(self, &self->moves[0]);
	} else
		pathfind(self, target->x_pos, target->y_pos);
}
