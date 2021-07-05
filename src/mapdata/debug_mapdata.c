#pragma bank 255

#include <gb/cgb.h>
#include <gb/incbin.h>

#include "debug_mapdata.h"
#include "include/map.h"

const void __at(255) __bank_debug_mapdata;

const mapdata debug_mapdata = {
	.tileset = debug_tileset,
	.colors = debug_colors,
	.metatiles = debug_metatiles,
	.nb_walls = 1,
	.wall_palette = debug_wall_palette
};

INCBIN(debug_tileset, res/gfx/tilesets/tree_tiles.2bpp)

const metatile debug_metatiles[] = {
	{
		.tiles = {0x12, 0x12, 0x12, 0x12},
		.attrs = {1, 1, 1, 1},
		.collision = NO_COLL
	},
	{
		.tiles = {0x04, 0x05, 0x0C, 0x0D},
		.attrs = {0, 0, 0, 0},
		.collision = WALL_COLL
	},
	{
		.tiles = {0x00, 0x01, 0x0C, 0x0D},
		.attrs = {0, 0, 0, 0},
		.collision = WALL_COLL
	},
	{
		.tiles = {0x00, 0x01, 0x0A, 0x0B},
		.attrs = {0, 0, 0, 0},
		.collision = WALL_COLL
	},
	{
		.tiles = {0x04, 0x05, 0x10, 0x11},
		.attrs = {0, 0, 0, 0},
		.collision = WALL_COLL
	},
	{
		.tiles = {2, 2, 2, 2},
		.attrs = {0, 0, 0, 0},
		.collision = EXIT_COLL
	},
};

const char debug_wall_palette[] = {1, 2, 3, 4};

const short debug_colors[] = {
	RGB(0x0A, 0x18, 0x0C), RGB(0x10, 0x0B, 0x07), RGB(0x00, 0x0B, 0x03), RGB(0x00, 0x04, 0x00),
	RGB(0x0A, 0x18, 0x0C), RGB(0x00, 0x0F, 0x00)
};