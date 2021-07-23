#pragma bank 2

#include <gb/cgb.h>
#include <gb/incbin.h>

#include "debug_mapdata.h"
#include "include/map.h"
#include "items/apple.h"

#define GRASS 6

#define TREE_PAL 0
#define GRASS_PAL 1

const void __at(2) __bank_debug_mapdata;

const mapdata debug_mapdata = {
	.tileset = debug_tileset,
	.colors = debug_colors,
	.metatiles = debug_metatiles,
	.nb_walls = 1,
	.wall_palette = debug_wall_palette,
	.exit_tile = 5,
	.item_table = item_table,
};

INCBIN(debug_tileset, res/gfx/tilesets/tree_tiles.2bpp)

const metatile debug_metatiles[] = {
	{
		.tiles = {GRASS, GRASS, GRASS, GRASS},
		.attrs = {GRASS_PAL, GRASS_PAL, GRASS_PAL, GRASS_PAL},
		.collision = NO_COLL
	},
	{
		.tiles = {4, 5, 11, 12},
		.attrs = {TREE_PAL, TREE_PAL, TREE_PAL, TREE_PAL},
		.collision = WALL_COLL
	},
	{
		.tiles = {0, 1, 11, 12},
		.attrs = {TREE_PAL, TREE_PAL, TREE_PAL, TREE_PAL},
		.collision = WALL_COLL
	},
	{
		.tiles = {0, 1, 9, 10},
		.attrs = {TREE_PAL, TREE_PAL, TREE_PAL, TREE_PAL},
		.collision = WALL_COLL
	},
	{
		.tiles = {4, 5, 13, 14},
		.attrs = {TREE_PAL, TREE_PAL, TREE_PAL, TREE_PAL},
		.collision = WALL_COLL
	},
	{
		.tiles = {7, 8, 16, 17},
		.attrs = {GRASS_PAL, GRASS_PAL, GRASS_PAL, GRASS_PAL},
		.collision = EXIT_COLL
	},
};

const char debug_wall_palette[] = {1, 2, 3, 4};

const short debug_colors[] = {
	RGB(10, 24, 12), RGB(16, 11, 7), RGB(0, 11, 3), RGB(0, 4, 0),
	RGB(10, 24, 12), RGB(0, 15, 0), RGB(0, 5, 0), RGB(0, 1, 0),
	RGB_BLACK, RGB_BLACK, RGB_BLACK, RGB_BLACK,
	RGB_BLACK, RGB_BLACK, RGB_BLACK, RGB_BLACK,
	RGB_BLACK, RGB_BLACK, RGB_BLACK, RGB_BLACK,
	RGB_BLACK, RGB_BLACK, RGB_BLACK, RGB_BLACK,
	RGB(10, 24, 12), RGB(31, 0, 0), RGB(16, 0, 0), RGB(8, 0, 0)
};

const struct item_weight item_table[] = {
	{.weight = 0x80, .bank = BANK(apple), .ptr = &apple_item, .pal = 6},
	{.weight = 0xFF, .ptr = NULL},
};