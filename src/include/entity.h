#pragma once

#include <gb/gb.h>
#include <stdbool.h>
#include <stdint.h>

#include "include/move.h"
#include "include/vec.h"

#define NB_ALLIES 3
#define BEGIN_ENEMIES 3
#define NB_ENEMIES 5
#define NB_ENTITIES 8

// The number of tiles each entity has allocated - do not edit.
#define NB_ENTITY_TILES 16
// Number of special tiles to be reloaded for animation - do not edit.
#define NB_SPECIAL_TILES 8
// The number of unique tiles in each direction - edit when adding new frames.
#define NB_UNIQUE_TILES 24

// 12 letters plus a 0 terminator.
#define ENTITY_NAME_LEN 12 + 1

#define PLAYER entities[0]

#define WALK_COST 1

enum special_frames {
	IDLE_FRAME = 0,
	IDLE_FRAME_2 = 1,
	WALK_FRAME = 2,
	WALK_FRAME_2 = 3,
	ATTACK_FRAME = 4,
	HURT_FRAME = 5,

	HIDE_FRAME = 255
};

struct leveled_move {
	uint8_t level;
	uint8_t bank;
	move_data *data;
};

enum entity_team {
	ENEMY_TEAM = 0,
	PLAYER_TEAM,
	ALLY_TEAM
};

/**
 * Contains constant data for entities.
 * 
 * @param metasprites	A pointer to the entity's metasprites.
 * @param graphics	A pointer to the entity's graphics.
 * @param colors	The entity's palette.
 * @param name		The entity's custom name.
 * @param base_xp	Used to calculate XP reward. XP reward is 
 * (base_xp + base_xp / 4 * level)
 * @param base_health	Used to calculat maximum health. Max health is 
 * (base_health + base_health / 8 * level)
 * @param leveled_move	A list of learnable moves sorted by level.
*/
typedef struct {
	const char *graphics;
	short *colors;
	const char *name;
	const uint8_t base_xp;
	const uint8_t base_health;
	const uint8_t base_fatigue;
	const uint8_t base_attack;
	const uint8_t base_defense;
	const struct leveled_move *level_moves;
} entity_data;

/**
 * @param data		A pointer to the entity's constant data. This is also 
 * used to verify that the entity exists.
 * @param bank		The bank of the entity's constant data.
 * @param x_pos		The x position of the entity on the grid.
 * @param y_pos 	The y position of the entity on the grid.
 * @param x_spr 	The x position of the entity's sprite
 * @param y_spr 	The y position of the entity's sprite
 * @param direction	Which direction the entity is facing.
 * @param prev_dir	Which direction the entity was facing the last time its
 * graphics were reloaded.
 * @param path_dir	Which direction the entity is choosing to pathfind in.
 * @param spr_frame	Which frame the sprite should display.
 * @param prev_frame	Which frame the sprite displayed the last time its 
 * graphics were reloaded.
 * @param moves		The entity's 4 available moves.
 * @param level		The entity's current level.
 * @param xp		The entity's progress to the next level.
*/
typedef struct {
	const entity_data *data;
	uint8_t bank;
	uint8_t x_pos;
	uint8_t y_pos;
	uint16_t x_spr;
	uint16_t y_spr;
	uint8_t direction;
	uint8_t prev_dir;
	int8_t path_dir;
	uint8_t spr_frame;
	uint8_t prev_frame;
	uint16_t health;
	uint16_t fatigue;
	char name[ENTITY_NAME_LEN];
	move moves[4];
	uint8_t level;
	uint16_t xp;
	uint8_t team;
} entity;

extern entity entities[NB_ENTITIES];
extern uint8_t move_speed;

void move_entities() NONBANKED;
uint16_t get_attack_bonus(entity *self) NONBANKED;
uint16_t get_defense_bonus(entity *self) NONBANKED;
uint16_t get_max_fatigue(entity *self) NONBANKED;
uint16_t get_max_health(entity *self) NONBANKED;
uint16_t get_xp_reward(entity *self) NONBANKED;

void attack_animation(entity *self) BANKED;
bool check_collision(uint8_t x, uint8_t y) BANKED;
int8_t check_entity_at(uint8_t x, uint8_t y) BANKED;
void defeat_animation(entity *self) BANKED;
void hurt_animation(entity *self) BANKED;
entity *new_entity(entity_data *data, uint8_t bank, uint8_t i, uint8_t x, 
		   uint8_t y, uint8_t level) BANKED;
void pathfind(entity *self, uint8_t target_x, uint8_t target_y) BANKED;
void ally_pathfind(entity *self, uint8_t target_x, uint8_t target_y) BANKED;
bool ally_try_step(entity *self) BANKED;
bool detection_trace(uint8_t self_x, uint8_t self_y, uint8_t target_x, 
		      uint8_t target_y, uint8_t distance) BANKED;
void pursue(entity *self, uint8_t start, uint8_t stop) BANKED;
void render_entities() BANKED;
entity *spawn_enemy(entity_data *data, uint8_t bank) BANKED;
bool try_step(entity *self, uint8_t dir) BANKED;
void use_melee_move(entity *self, move *self_move);
void reload_entity_graphics(uint8_t i) NONBANKED;

inline uint16_t get_xp_threshold(uint8_t level) {
	return level * 256;
}