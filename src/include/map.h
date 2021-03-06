#pragma once

#include <gb/gb.h>
#include <stdbool.h>
#include <stdint.h>

#include "include/entity.h"
#include "include/vec.h"

enum collision {
	NO_COLL = 0,
	WALL_COLL,
	EXIT_COLL
};

typedef struct {
	unsigned char tiles[4];
	unsigned char attrs[4];
	unsigned char collision;
} metatile;

struct enemy_spawn_data {
	const entity_data *ptr;
	const uint8_t bank;
	const uint8_t level;
};

struct item_weight {
	uint8_t weight;
	uint8_t bank;
	void *ptr;
	uint8_t pal;
};

/**
 * Data pertaining to the currently loaded level.
 * 
 * @param tileset		Array of 128 tiles for the map to use.
 * @param metatiles		Array of metatiles that contian tiles, 
 * attributes, and collision. Packs 9 bytes into 1!
 * @param post_process		Post-processing function to convert the raw map
 * into metatiles.
 * @param nb_walls		The number of wall variants in the wall palette.
 * Used by the post-processing function.
 * @param wall_palette		An array of wall variants. The format depends on
 * the map's post-processing function.
 * @param completion_flag	SRAM flag to set after successfully completing
 * the level.
 * @param final_floor		Successfully exit the dungeon when reaching this
 * floor.
 * @param enemy_levels		4 possible levels for entities in the level.
 * @param enemy_list		A list of 8 enemies that can spawn in the level.
 * NULL entries will reduce the spawn rate.
*/
typedef struct {
	const char *tileset;
	short *colors;
	const metatile *metatiles;
	const void *post_process;
	const char *wall_palette;
	const uint8_t exit_tile;
	const struct item_weight *item_table;
	const uint8_t completion_flag;
	const uint8_t final_floor;
	const uint8_t enemy_levels[4];
	const struct enemy_spawn_data enemy_list[8];
} mapdata;

// Y, X order.
extern uint8_t map[64][64];
extern uvec16 camera;
extern const mapdata *current_mapdata;
extern uint8_t current_mapdata_bank;

void draw_tile(uint8_t x, uint8_t y) NONBANKED;
void generate_enemy() NONBANKED;void generate_enemy() NONBANKED;
void update_camera(uint16_t x, uint16_t y) BANKED;
uint8_t get_collision(uint16_t x, uint16_t y) NONBANKED;
void force_render_map() NONBANKED;
void reload_mapdata() NONBANKED;

void generate_map() BANKED;
void create_new_floor() BANKED;
bool is_open_location(uint8_t x, uint8_t y) BANKED;