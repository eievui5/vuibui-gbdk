#pragma bank 255

#include <gb/cgb.h>
#include <gb/incbin.h>

#include "field_mapdata.h"
#include "include/map.h"
#include "items/apple.h"

#define TREE_PAL 0
#define GRASS_PAL 1
#define STAIR_PAL 2

const void __at(255) __bank_field_mapdata;

const mapdata field_mapdata = {
	.tileset = field_tileset,
	.colors = field_colors,
	.metatiles = field_metatiles,
	.nb_walls = 1,
	.wall_palette = field_wall_palette,
	.exit_tile = 5,
	.item_table = field_item_table,
};

INCBIN(field_tileset, res/gfx/tilesets/field_tiles.2bpp)

const metatile field_metatiles[] = {
	{
		.tiles = {10, 14, 14, 10},
		.attrs = {GRASS_PAL, GRASS_PAL, GRASS_PAL, GRASS_PAL},
		.collision = NO_COLL
	},
	{
		.tiles = {0, 1, 12, 13},
		.attrs = {TREE_PAL, TREE_PAL, TREE_PAL, TREE_PAL},
		.collision = WALL_COLL
	},
	{
		.tiles = {8, 9, 12, 13},
		.attrs = {TREE_PAL, TREE_PAL, TREE_PAL, TREE_PAL},
		.collision = WALL_COLL
	},
	{
		.tiles = {8, 9, 4, 5},
		.attrs = {TREE_PAL, TREE_PAL, TREE_PAL, TREE_PAL},
		.collision = WALL_COLL
	},
	{
		.tiles = {0, 1, 4, 5},
		.attrs = {TREE_PAL, TREE_PAL, TREE_PAL, TREE_PAL},
		.collision = WALL_COLL
	},
	{
		.tiles = {2, 3, 6, 7},
		.attrs = {STAIR_PAL, STAIR_PAL, STAIR_PAL, STAIR_PAL},
		.collision = EXIT_COLL
	},
};

const char field_wall_palette[] = {1, 2, 3, 4};

const short field_colors[] = {
	RGB(15, 24, 12), RGB(4, 15, 0), RGB(3, 8, 3), RGB(0, 4, 0),
	RGB(15, 24, 12), RGB(8, 15, 0), RGB(0, 5, 0), RGB(0, 1, 0),
	RGB(15, 24, 12), RGB(12, 10, 0), RGB(8, 6, 0), RGB(4, 3, 0),
	RGB_BLACK, RGB_BLACK, RGB_BLACK, RGB_BLACK,
	RGB_BLACK, RGB_BLACK, RGB_BLACK, RGB_BLACK,
	RGB_BLACK, RGB_BLACK, RGB_BLACK, RGB_BLACK,
	RGB(15, 24, 12), RGB(31, 0, 0), RGB(16, 0, 0), RGB(8, 0, 0)
};

const struct item_weight field_item_table[] = {
	{.weight = 0x80, .bank = BANK(apple), .ptr = &apple_item, .pal = 6},
	{.weight = 0xFF, .ptr = NULL},
};