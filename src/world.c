#pragma bank 255

#include <gb/cgb.h>
#include <stdint.h>

#include "include/dir.h"
#include "include/game.h"
#include "include/rendering.h"
#include "include/save.h"
#include "include/vec.h"
#include "include/world.h"

#include "entities/luvui.h"
#include "mapdata/debug_mapdata.h"
#include "mapdata/field_mapdata.h"
INCBIN(crater_graphics, res/gfx/maps/crater.2bpp)
INCBIN_EXTERN(crater_graphics)
INCBIN(crater_map, res/gfx/maps/crater.map)
INCBIN_EXTERN(crater_map)

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
map_node *current_mapnode = &crater_house;
uvec8 worldmap_pos;
uint8_t worldmap_direction;

void select_node(enum Direction dir) {
	if (!current_mapnode->connections[dir])
		return;
	if (!((map_node *) current_mapnode->connections[dir])->unlock_flag ||
	    get_sram_flag(((map_node *) current_mapnode->connections[dir])->unlock_flag)) {
		current_mapnode = current_mapnode->connections[dir];
		worldmap_direction = dir;
	}
}

void init_worldmap() BANKED
{
	fx_mode = NO_UI;
	lcdc_buffer = \
		LCDC_ENABLE | LCDC_BG_ENABLE | LCDC_WINDOW_ENABLE | \
		LCDC_WINDOW_SCRN1 | LCDC_OBJ_ENABLE | LCDC_OBJ_16;
	cgb_compatibility();
	set_bkg_data(0x00, 0x7F, crater_graphics);
	set_bkg_tiles(0, 0, 20, 14, crater_map);
	banked_vmemcpy((void *)(0x8000), 64, &gfx_luvui[24 * 16 * DIR_DOWN], BANK(gfx_luvui));
	worldmap_pos.x = current_mapnode->x * 8;
	worldmap_pos.y = current_mapnode->y * 8;
	shadow_OAM[0].tile = 0;
	shadow_OAM[1].tile = 2;
	shadow_OAM[0].y = worldmap_pos.y + 8;
	shadow_OAM[0].x = worldmap_pos.x + 4;
	shadow_OAM[1].y = worldmap_pos.y + 8;
	shadow_OAM[1].x = worldmap_pos.x + 12;
}

void simulate_worldmap() BANKED
{
	if (cur_keys & J_A && current_mapnode->type == DUNGEON_NODE) {
		current_mapdata = current_mapnode->level;
		current_mapdata_bank = current_mapnode->bank;
		game_state = DUNGEON_STATE;
	} else if (cur_keys & J_UP) {
		select_node(DIR_UP);
	} else if (cur_keys & J_RIGHT) {
		select_node(DIR_RIGHT);
	} else if (cur_keys & J_DOWN) {
		select_node(DIR_DOWN);
	} else if (cur_keys & J_LEFT) {
		select_node(DIR_LEFT);
	}

	if (worldmap_pos.x != current_mapnode->x * 8 ||
		worldmap_pos.y != current_mapnode->y * 8) {
		while (worldmap_pos.x != current_mapnode->x * 8 ||
			worldmap_pos.y != current_mapnode->y * 8) {
			move_direction((vec8 *)&worldmap_pos, worldmap_direction);
			move_direction((vec8 *)&worldmap_pos, worldmap_direction);
			shadow_OAM[0].y = worldmap_pos.y + 8;
			shadow_OAM[0].x = worldmap_pos.x + 4;
			shadow_OAM[1].y = worldmap_pos.y + 8;
			shadow_OAM[1].x = worldmap_pos.x + 12;
			wait_vbl_done();
		}
	}
}