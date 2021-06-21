#pragma once

#include "int.h"

#define NB_ALLIES 3
#define NB_ENEMIES 5
#define NB_ENTITIES 8

/**
 * Contians constant data for entities.
 * 
 * @param metasprites	A pointer to the entity's metasprites.
*/
typedef struct {
	const char *metasprites;
} entity_data;

/**
 * @param data		A pointer to the entity's constant data. This is also 
 * used to verify that the entity exists.
 * @param bank		The bank of the entity's constant data.
 * @param x_pos		The x position of the entity on the grid.
 * @param y_pos 	The y position of the entity on the grid.
 * @param x_spr 	The x position of the entity's sprite
 * @param y_spr 	The y position of the entity's sprite
*/
typedef struct {
	const entity_data *data;
	u8 bank;
	u16 x_pos;
	u16 y_pos;
	u16 x_spr;
	u16 y_spr;
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

void move_entities();
entity *check_collision(u8 ignore, u16 x, u16 y);
void render_entities();