#pragma once

#include <gb/gb.h>
#include "include/int.h"

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


typedef struct {
	const char *tileset;
	const metatile *metatiles;
} mapdata;

// Y, X order.
extern u8 map[64][64];
extern u16 camera_x;
extern u16 camera_y;

void update_camera(u16 x, u16 y) NONBANKED;
u8 get_collision(u16 x, u16 y) NONBANKED;
void force_render_map() NONBANKED;
void load_mapdata(mapdata *data, u8 bank) NONBANKED;

void generate_map() BANKED;