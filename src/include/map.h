#pragma once

#include <gb/gb.h>

#include "include/int.h"
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

/**
 * Data pertaining to the currently loaded map's visuals.
 * 
 * @param tileset	Array of 128 tiles for the map to use.
 * @param metatiles	Array of metatiles that contian tiles, attributes, and
 * collision. Packs 9 bytes into 1!
 * @param post_process	Post-processing function to convert the raw map into
 * metatiles.
 * @param nb_walls	The number of wall variants in the wall palette. Used by
 * the post-processing function.
 * @param wall_palette	And array of wall variants. The format depends on the
 * map's post-processing function.
*/
typedef struct {
	const char *tileset;
	short *colors;
	const metatile *metatiles;
	const void *post_process;
	const u8 nb_walls;
	const char *wall_palette;
} mapdata;

// Y, X order.
extern u8 map[64][64];
extern uvec16 camera;

void update_camera(u16 x, u16 y) NONBANKED;
u8 get_collision(u16 x, u16 y) NONBANKED;
void force_render_map() NONBANKED;
void load_mapdata(mapdata *data, u8 bank) NONBANKED;

void generate_map() BANKED;