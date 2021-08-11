#pragma bank 255

#include <gb/cgb.h>
#include <stdint.h>

#include "include/dir.h"
#include "include/game.h"
#include "include/rendering.h"
#include "include/vec.h"
#include "include/world.h"

#include "entities/luvui.h"
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
	.connections = {&crater_intersection, &crater_house, NULL, NULL},
	.level = &field_mapdata,
	.bank = BANK(field_mapdata),
};
const map_node crater_intersection = {
	.x = 2,
	.y = 4,
	.type = FORK_NODE,
	.connections = {NULL, NULL, &crater_woodland, NULL},
};
map_node *current_mapnode = (map_node *) &crater_woodland;
uvec8 worldmap_pos;
uint8_t worldmap_direction;

// Returns the type of the player's selected node.
uint8_t simulate_worldmap() BANKED
{
	LCDC_REG = lcdc_buffer = \
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
	while (1) {
		if (cur_keys & J_A && current_mapnode->type == DUNGEON_NODE) {

			current_mapdata = current_mapnode->level;
			current_mapdata_bank = current_mapnode->bank;
			return DUNGEON_NODE;
		} else if (cur_keys & J_UP && current_mapnode->connections[DIR_UP]) {
			current_mapnode = current_mapnode->connections[DIR_UP];
			worldmap_direction = DIR_UP;
		} else if (cur_keys & J_RIGHT && current_mapnode->connections[DIR_RIGHT]) {
			current_mapnode = current_mapnode->connections[DIR_RIGHT];
			worldmap_direction = DIR_RIGHT;
		} else if (cur_keys & J_DOWN && current_mapnode->connections[DIR_DOWN]) {
			current_mapnode = current_mapnode->connections[DIR_DOWN];
			worldmap_direction = DIR_DOWN;
		} else if (cur_keys & J_LEFT && current_mapnode->connections[DIR_LEFT]) {
			current_mapnode = current_mapnode->connections[DIR_LEFT];
			worldmap_direction = DIR_LEFT;
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
		wait_vbl_done();
	}
}