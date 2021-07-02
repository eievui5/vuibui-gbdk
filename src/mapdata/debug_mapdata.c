#pragma bank 255

#include "include/map.h"

const void __at(255) __bank_debug_mapdata;

const mapdata debug_mapdata = {
	.tileset = debug_tileset,
	.metatiles = debug_metatiles,
};

const unsigned char debug_tileset[] = {
	0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00,
	0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00,
	0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF,
	0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF,
	0xFF, 0xFF, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81,
	0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0xFF, 0xFF,
};

const metatile debug_metatiles[] = {
	{
		.tiles = {0, 0, 0, 0},
		.attrs = {0, 0, 0, 0},
		.collision = NO_COLL
	},
	{
		.tiles = {1, 1, 1, 1},
		.attrs = {0, 0, 0, 0},
		.collision = WALL_COLL
	},
	{
		.tiles = {2, 2, 2, 2},
		.attrs = {0, 0, 0, 0},
		.collision = EXIT_COLL
	},
};