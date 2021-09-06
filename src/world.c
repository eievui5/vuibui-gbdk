#pragma bank 255

#include <gb/cgb.h>
#include <gb/incbin.h>
#include <stdint.h>
#include <string.h>

#include "include/cutscene.h"
#include "include/dir.h"
#include "include/hud.h"
#include "include/game.h"
#include "include/map.h"
#include "include/rendering.h"
#include "include/save.h"
#include "include/vec.h"
#include "include/world.h"
#include "libs/vwf.h"

#include "entities/luvui.h"
#include "worldmaps/crater.h"

INCBIN(worldmap_ui_gfx, res/gfx/maps/worldmap_ui.2bpp)
INCBIN(worldmap_markers, res/gfx/maps/worldmap_markers.2bpp)
INCBIN_EXTERN(worldmap_ui_gfx)
INCBIN_EXTERN(worldmap_markers)

#define PLAYER_MARKPAL 0
#define COMPLETE_MARKPAL 1
#define INCOMPLETE_MARKPAL 2

#define FONT_PTR ((char*) 0x8900)
#define FONT_TILE 0x90u

const short marker_pals[] = {
	RGB_WHITE, RGB_BLUE, RGB_DARKBLUE, RGB_BLACK,
	RGB_WHITE, RGB_RED, RGB_DARKRED, RGB_BLACK,
};

uint8_t current_worldmap_bank = BANK(crater);
world_map *current_worldmap = &crater_worldmap;
map_node *current_mapnode = &crater_house;
uvec8 worldmap_pos;
uint8_t worldmap_direction = DIR_DOWN;
bool redraw_text_flag;

void render_world_objects() NONBANKED
{
	static uint8_t anim_timer = 0;
	
	if (worldmap_pos.x != current_mapnode->x * 8 ||
	    worldmap_pos.y != current_mapnode->y * 8) {
		draw_static_entity(PLAYER.data, PLAYER.bank, worldmap_direction,
				   WALK_FRAME + (anim_timer++ & 0b10000 ? 1 : 0),
				   (char*) 0x8000, PLAYER_MARKPAL);
	} else {
		draw_static_entity(PLAYER.data, PLAYER.bank, DIR_DOWN, anim_timer++ & 0b10000 ? 1 : 0,
			(char*) 0x8000, PLAYER_MARKPAL);
	}
	reset_oam();
	uint8_t *oam_pointer = (uint8_t *) shadow_OAM;

	*oam_pointer++ = worldmap_pos.y + 8;
	*oam_pointer++ = worldmap_pos.x + 4;
	*oam_pointer++ = 0;
	*oam_pointer++ = PLAYER_MARKPAL;
	*oam_pointer++ = worldmap_pos.y + 8;
	*oam_pointer++ = worldmap_pos.x + 12;
	*oam_pointer++ = 2;
	*oam_pointer++ = PLAYER_MARKPAL;

	uint8_t temp_bank = _current_bank;
	SWITCH_ROM_MBC1(current_worldmap_bank);
	for (uint8_t i = 0; current_worldmap->nodes[i]; i++) {
		if (current_mapnode == current_worldmap->nodes[i] &&
		    worldmap_pos.x == current_mapnode->x * 8 &&
		    worldmap_pos.y == current_mapnode->y * 8)
			continue;
		*oam_pointer++ = current_worldmap->nodes[i]->y * 8 + 16;
		*oam_pointer++ = current_worldmap->nodes[i]->x * 8 + 8;

		if (!current_worldmap->nodes[i]->unlock_flag ||
		    get_sram_flag(current_worldmap->nodes[i]->unlock_flag)) {
			// Check for overrides.
			uint8_t marker = current_worldmap->nodes[i]->unlocked_marker;
			if (!marker) {
				switch (current_worldmap->nodes[i]->type) {
				case DUNGEON_NODE:
					marker = UNLOCKED_MARKER;
					break;
				default:
					marker = STUB_MARKER;
					break;
				}
			}
			*oam_pointer++ = marker;
		} else {
			// Check for overrides.
			uint8_t marker = current_worldmap->nodes[i]->locked_marker;
			if (!marker) {
				switch (current_worldmap->nodes[i]->type) {
				case DUNGEON_NODE:
					marker = LOCKED_MARKER;
					break;
				default:
					marker = STUB_MARKER;
					break;
				}
			}
			*oam_pointer++ = marker;
		}
		if (!current_worldmap->nodes[i]->complete_flag ||
		    get_sram_flag(current_worldmap->nodes[i]->complete_flag)) {
			// Check for overrides.
			uint8_t attr = current_worldmap->nodes[i]->complete_attr;
			if (!attr) {
				switch (current_worldmap->nodes[i]->type) {
				default:
					attr = COMPLETE_MARKPAL;
					break;
				}
			}
			*oam_pointer++ = attr;
		} else {
			// Check for overrides.
			uint8_t attr = current_worldmap->nodes[i]->incomplete_attr;
			if (!attr) {
				switch (current_worldmap->nodes[i]->type) {
				default:
					attr = INCOMPLETE_MARKPAL | OAM_DMG_PAL1;
					break;
				}
			}
			*oam_pointer++ = attr;
		}
	}
	SWITCH_ROM_MBC1(temp_bank);
}

void select_node(enum Direction dir) NONBANKED
{
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
	scr_pos.x = 0;
	scr_pos.y = 0;
	redraw_text_flag = true;
	fx_mode = NO_UI;
	lcdc_buffer = \
		LCDC_ENABLE | LCDC_BG_ENABLE | LCDC_WINDOW_ENABLE | \
		LCDC_WINDOW_SCRN1 | LCDC_OBJ_ENABLE | LCDC_OBJ_16;
	// Wait for all FX to finish after changing UI mode & LCDC.
	wait_vbl_done();
	
	uint8_t temp_bank = _current_bank;
	SWITCH_ROM_MBC1(current_worldmap_bank);
	banked_vsetmap((char*) 0x9800, 20, 14, crater_map, BANK(crater_map));
	if (_cpu == CGB_TYPE) {
		set_bkg_palette(0, 7, current_worldmap->pals);
		set_bkg_palette(7, 1, current_ui_pal.colors);
		VBK_REG = 1;
		banked_vsetmap((char*) 0x9800, 20, 14, crater_attr, BANK(crater_map));
		vmemset((char*) 0x99C0, 7, 0x74);
		VBK_REG = 0;
	}
	banked_vmemcpy((char*) 0x9000, 0x800, crater_graphics, BANK(crater_graphics));
	worldmap_pos.x = current_mapnode->x * 8;
	worldmap_pos.y = current_mapnode->y * 8;

	SWITCH_ROM_MBC1(BANK(worldmap_ui_gfx));
	vmemcpy((char*) 0x8800, SIZE(worldmap_ui_gfx), worldmap_ui_gfx);
	vmemset((char*) 0x99C0, 0x80, 20);
	vmemset((char*) 0x99E0, 0x81, 32 * 3);
	vmemcpy((char*) 0x8F00, SIZE(worldmap_markers), worldmap_markers);
	set_sprite_palette(1, 7, marker_pals);
	SWITCH_ROM_MBC1(temp_bank);

	render_world_objects();
}

void simulate_worldmap() NONBANKED
{
	uint8_t temp_bank = _current_bank;
	SWITCH_ROM_MBC1(current_worldmap_bank);

	if (worldmap_pos.x != current_mapnode->x * 8 ||
	    worldmap_pos.y != current_mapnode->y * 8) {
		move_direction((vec8 *)&worldmap_pos, worldmap_direction);
		redraw_text_flag = true;
	}
	if (worldmap_pos.x == current_mapnode->x * 8 &&
	    worldmap_pos.y == current_mapnode->y * 8) {
		if (redraw_text_flag) {
			vmemset(FONT_PTR, 0, 0x100);
			if (current_mapnode->name) {
				vwf_activate_font(0);
				vwf_draw_text(1, 15, (char*) 0x9800, FONT_TILE,
					current_mapnode->name);
			}
			redraw_text_flag = false;
		}
		if (new_keys & J_A) {
			switch (current_mapnode->type) {
			case DUNGEON_NODE:
				current_mapdata = current_mapnode->level;
				current_mapdata_bank = current_mapnode->bank;
				game_state = DUNGEON_STATE;
				break;
			case CUTSCENE_NODE:
				cur_script = current_mapnode->level;
				cur_script_bank = current_mapnode->bank;
				game_state = CUTSCENE_STATE;
				break;
			}
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
	render_world_objects();
	SWITCH_ROM_MBC1(temp_bank);
}