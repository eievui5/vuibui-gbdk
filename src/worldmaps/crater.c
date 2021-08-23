#pragma bank 255

#include <gb/cgb.h>
#include <gb/incbin.h>

#include "include/bank.h"
#include "include/cutscene.h"
#include "include/dir.h"
#include "include/entity.h"
#include "include/world.h"

#include "crater.h"
#include "entities/entity_list.h"
#include "mapdata/debug_mapdata.h"
#include "mapdata/field_mapdata.h"

INCBIN(crater_graphics, res/gfx/maps/crater.2bpp)
INCBIN(crater_map, res/gfx/maps/crater.map)
INCBIN(crater_attr, src/gfx/maps/crater.attr)
DEF_BANK(crater)

SCRIPT(debug_script) {
	LOAD_ENTITY(0, BANK(aris), &aris_entity)
	SET_ENTITY_POS(0, 32, 32)
	SET_ENTITY_DIR(0, DIR_RIGHT)
	RENDER_ENTITY(0)
	JUMP(&debug_script_loop)
}; SCRIPT(debug_script_loop) {
	YIELD()
	MOVE_ENTITY(0, 64, 64)
	//ANIMATE_ENTITY(0, 0b10000, WALK_FRAME, WALK_FRAME_2)
	JUMP(&debug_script_loop)
};

const map_node crater_house = {
	.x = 10,
	.y = 12,
	.type = CUTSCENE_NODE,
	.bank = BANK(crater),
	.level = &debug_script,
	.connections = {NULL, NULL, NULL, &crater_woodland},
};
const map_node crater_woodland = {
	.name = "Crater Woodland",
	.x = 2,
	.y = 12,
	.type = DUNGEON_NODE,
	.connections = {&crater_fields, &crater_house, NULL, NULL},
	.level = &debug_mapdata,
	.bank = BANK(debug_mapdata),

	.complete_flag = FLAG_WORLD_1_1,
};
const map_node crater_fields = {
	.name = "Crater Fields",
	.x = 2,
	.y = 5,
	.type = DUNGEON_NODE,
	.connections = {NULL, &crater_lake, &crater_woodland, NULL},
	.level = &field_mapdata,
	.bank = BANK(field_mapdata),

	.unlock_flag = FLAG_WORLD_1_1,
	.complete_flag = FLAG_WORLD_1_2,
};
const map_node crater_town_2 = {
	.x = 2,
	.y = 3,
	.type = FORK_NODE,
	.connections = {NULL, NULL, &crater_fields, NULL},

	.unlock_flag = FLAG_WORLD_1_2,
};
const map_node crater_lake = {
	.x = 6,
	.y = 5,
	.type = FORK_NODE,
	.connections = {NULL, &crater_lake_turn, NULL, &crater_fields},

	.unlock_flag = FLAG_WORLD_1_2,
	.complete_flag = FLAG_WORLD_1_2,
};
const map_node crater_lake_turn = {
	.x = 9,
	.y = 5,
	.type = FORK_NODE,
	.connections = {NULL, NULL, NULL, &crater_lake},
};

const short crater_pals[] = {
	RGB(11, 27, 13), RGB(11, 14, 19), RGB(10, 5, 5), RGB_BLACK,
	RGB(11, 27, 13), RGB(6, 15, 8), RGB(10, 5, 5), RGB_BLACK,
	RGB(11, 27, 13), RGB(16, 15, 10), RGB(10, 5, 5), RGB_BLACK,
	RGB(11, 27, 13), RGB(17, 10, 8), RGB(10, 5, 5), RGB_BLACK,
	RGB(11, 27, 13), RGB(13, 10, 10), RGB(11, 8, 8), RGB_BLACK,
};

const world_map crater_worldmap = {
	.name = "Crater",
	.pals = crater_pals,
	.nodes = {
		&crater_house,
		&crater_woodland,
		&crater_fields,
		&crater_town_2,
		&crater_lake,
		&crater_lake_turn,
		NULL
	}
};