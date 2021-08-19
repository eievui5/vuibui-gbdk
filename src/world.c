#pragma bank 255

#include <gb/cgb.h>
#include <gb/incbin.h>
#include <stdint.h>

#include "include/dir.h"
#include "include/game.h"
#include "include/map.h"
#include "include/rendering.h"
#include "include/save.h"
#include "include/vec.h"
#include "include/world.h"

#include "entities/luvui.h"
#include "mapdata/debug_mapdata.h"
#include "mapdata/field_mapdata.h"
#include "worldmaps/crater.h"

INCBIN(worldmap_ui_gfx, res/gfx/maps/worldmap_ui.2bpp)
INCBIN(worldmap_ui_map, res/gfx/maps/worldmap_ui.map)
INCBIN_EXTERN(worldmap_ui_gfx)
INCBIN_EXTERN(worldmap_ui_map)

uint8_t current_worldmap_bank = BANK(crater);
world_map *current_worldmap = &crater_worldmap;
map_node *current_mapnode = &crater_house;
uvec8 worldmap_pos;
uint8_t worldmap_direction;

void select_node(enum Direction dir) NONBANKED {
	if (!current_mapnode->connections[dir])
		return;
	if (!((map_node *) current_mapnode->connections[dir])->unlock_flag ||
	    get_sram_flag(((map_node *) current_mapnode->connections[dir])->unlock_flag)) {
		current_mapnode = current_mapnode->connections[dir];
		worldmap_direction = dir;
	}
}

void init_worldmap() NONBANKED
{
	uint8_t temp_bank = _current_bank;
	SWITCH_ROM_MBC1(current_worldmap_bank);
	fx_mode = NO_UI;
	lcdc_buffer = \
		LCDC_ENABLE | LCDC_BG_ENABLE | LCDC_WINDOW_ENABLE | \
		LCDC_WINDOW_SCRN1 | LCDC_OBJ_ENABLE | LCDC_OBJ_16;
	cgb_compatibility();
	banked_vsetmap((void *) 0x9800, 20, 14, crater_map, BANK(crater_map));
	banked_vmemcpy((void *) 0x9000, 0x800, crater_graphics, BANK(crater_graphics));
	banked_vmemcpy((void *) 0x8000, 64, &gfx_luvui[24 * 16 * DIR_DOWN], BANK(gfx_luvui));
	worldmap_pos.x = current_mapnode->x * 8;
	worldmap_pos.y = current_mapnode->y * 8;
	shadow_OAM[0].tile = 0;
	shadow_OAM[1].tile = 2;
	shadow_OAM[0].y = worldmap_pos.y + 8;
	shadow_OAM[0].x = worldmap_pos.x + 4;
	shadow_OAM[1].y = worldmap_pos.y + 8;
	shadow_OAM[1].x = worldmap_pos.x + 12;

	SWITCH_ROM_MBC1(BANK(worldmap_ui_gfx));
	vmemcpy((void *) 0x8800, SIZE(worldmap_ui_gfx), worldmap_ui_gfx);
	vsetmap((void *) 0x99C0, 20, 4, worldmap_ui_map);
	SWITCH_ROM_MBC1(temp_bank);
}

void simulate_worldmap() NONBANKED
{
	uint8_t temp_bank = _current_bank;
	SWITCH_ROM_MBC1(current_worldmap_bank);

	if (worldmap_pos.x != current_mapnode->x * 8 ||
		worldmap_pos.y != current_mapnode->y * 8) {
		move_direction((vec8 *)&worldmap_pos, worldmap_direction);
		move_direction((vec8 *)&worldmap_pos, worldmap_direction);
		shadow_OAM[0].y = worldmap_pos.y + 8;
		shadow_OAM[0].x = worldmap_pos.x + 4;
		shadow_OAM[1].y = worldmap_pos.y + 8;
		shadow_OAM[1].x = worldmap_pos.x + 12;
	} else {
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
	}
	SWITCH_ROM_MBC1(temp_bank);
}