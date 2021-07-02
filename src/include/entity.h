#pragma once

#include <gb/gb.h>
#include <stdbool.h>

#include "include/int.h"

#define NB_ALLIES 3
#define NB_ENEMIES 5
#define NB_ENTITIES 8

#define NB_ENTITY_TILES 16

/**
 * Contains constant data for entities.
 * 
 * @param metasprites	A pointer to the entity's metasprites.
 * @param graphics	A pointer to the entity's graphics.
*/
typedef struct {
	const char *metasprites;
	const char *graphics;
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
 * @param spr_frame	Which frame the sprite should display.
 * @param prev_frame	Which frame the sprite displayed the last time its 
 * graphics were reloaded.
*/
typedef struct {
	const entity_data *data;
	u8 bank;
	u8 x_pos;
	u8 y_pos;
	u16 x_spr;
	u16 y_spr;
	u8 direction;
	u8 prev_dir;
	u8 spr_frame;
	u8 prev_frame;
} entity;

/**
 * Used to organize the entity array into players, allies, and enemies.
 * 
 * @param player	The player entity.
 * @param allies	The player's team, including the player entity.
 * @param enemies	The enemy entities, placed after the allies.
 * @param array		The entire entity array, including both allies and 
 * enemies.
*/
typedef union {
	struct {
		union {
			entity player;
			entity allies[NB_ALLIES];
		};
		entity enemies[NB_ENEMIES];
	};
	entity array[NB_ENTITIES];
} entity_array;

extern entity_array entities;

void render_entities() NONBANKED;
void move_entities() NONBANKED;

bool try_step(u8 i, u8 dir) BANKED;
bool check_collision(u8 ignore, u8 x, u8 y) BANKED;