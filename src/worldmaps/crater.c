#pragma bank 255

#include <gb/incbin.h>

#include "include/bank.h"
#include "include/world.h"

#include "crater.h"
#include "mapdata/debug_mapdata.h"
#include "mapdata/field_mapdata.h"

INCBIN(crater_graphics, res/gfx/maps/crater.2bpp)
INCBIN(crater_map, res/gfx/maps/crater.map)
DEF_BANK(crater)

const map_node crater_house = {
	.x = 10,
	.y = 12,
	.type = FORK_NODE,
	.connections = {NULL, NULL, NULL, &crater_woodland},
};
const map_node crater_woodland = { 
	.x = 2,
	.y = 12,
	.type = DUNGEON_NODE,
	.connections = {&crater_fields, &crater_house, NULL, NULL},
	.level = &debug_mapdata,
	.bank = BANK(debug_mapdata),
};
const map_node crater_fields = {
	.x = 2,
	.y = 7,
	.type = DUNGEON_NODE,
	.connections = {NULL, NULL, &crater_woodland, NULL},
	.level = &field_mapdata,
	.bank = BANK(field_mapdata),
	.unlock_flag = FLAG_WORLD_1_1,
};

const world_map crater_worldmap = {
	.name = "Crater",
	.nodes = {
		&crater_house,
		&crater_woodland,
		&crater_fields,
		NULL
	}
};