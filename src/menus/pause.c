#pragma bank 255

#include <stdbool.h>
#include <string.h>

#include "include/bank.h"
#include "include/entity.h"
#include "include/hud.h"
#include "include/game.h"
#include "include/hardware.h"
#include "include/int.h"
#include "include/item.h"
#include "include/map.h"
#include "include/rendering.h"
#include "libs/vwf.h"

#define SWIPE_SPEED 12
#define BLANK_TILE 0x8E
#define FONT_TILE 0x8F
#define SUBFONT_TILE 0xA6u
#define DESCFONT_TILE (SUBFONT_TILE + 56u)
#define CURSOR_TILE 0x00
#define CURSOR_SPEED 4
#define NB_OPTIONS (6-1)
#define SUBMENU_SLIDE_POS 0x60
#define SUBMENU_SLIDE_SPEED 8

enum choice {
	RETURN_CHOICE,
	ITEMS_CHOICE,
	PARTY_CHOICE,
	SAVE_CHOICE,
	OPTIONS_CHOICE,
	ESCAPE_CHOICE
};

INCBIN(paw_print, res/gfx/ui/paw_mark.2bpp)
INCBIN_EXTERN(paw_print)
INCBIN(paw_print_map, res/gfx/ui/paw_mark.map)
INCBIN_EXTERN(paw_print_map)
INCBIN(paw_cursor, res/gfx/ui/paw_cursor.h.2bpp)
INCBIN_EXTERN(paw_cursor)
DEF_BANK(pause_menu)

const char pause_text[] = \
"Return\n\nItems\n\nParty\n\nSave\n\nOptions\n\nEscape!";
const char apple_text[] = \
"Apple\nApple\nApple\nApple\nApple\nApple\nApple\nApple\n";

void draw_inventory(u8 start) NONBANKED
{
	u8 temp_bank = _current_bank;

	for (u8 i = 0; i < 8; i++) {
		vmemset((void *)(0x9C38 + i * 32), BLANK_TILE, 8);
	}
	for (u8 i = 0; (start < INVENTORY_SIZE) && (i < 8); i++, start++) {
		if (inventory[start].data) {
			SWITCH_ROM_MBC1(inventory[start].bank);
			vwf_draw_text(24, 1 + i, SUBFONT_TILE + i * 8, 
				      inventory[start].data->name);
		}
	}

	SWITCH_ROM_MBC1(temp_bank);
}

void draw_description(u8 i) NONBANKED
{
	if (!inventory[i].data)
		return;
	u8 temp_bank = _current_bank;
	SWITCH_ROM_MBC1(inventory[i].bank);
	vwf_draw_text(21, 13, DESCFONT_TILE,
		      inventory[i].data->desc);
	SWITCH_ROM_MBC1(temp_bank);
}

u8 item_menu() BANKED
{
	u8 cursor_pos = 0;
	u8 cursor_spr = 20;
	u8 base_item = 0;
	bool redraw_flag = true;

	draw_inventory(0);

	shadow_OAM[0].x = 22u * 8u + 4u;
	shadow_OAM[1].x = 22u * 8u + 12u;
	shadow_OAM[0].y = 20;
	shadow_OAM[1].y = 20;
	while (SCX_REG < SUBMENU_SLIDE_POS) {
		wait_vbl_done();
		SCX_REG += SUBMENU_SLIDE_SPEED;
		shadow_OAM[0].x -= SUBMENU_SLIDE_SPEED;
		shadow_OAM[1].x -= SUBMENU_SLIDE_SPEED;
	}
	SCX_REG = SUBMENU_SLIDE_POS;

	while (1) {

		// Handle new inputs.
		update_input();
		switch (new_keys) {
		case J_START: case J_B:
			goto exit;
		case J_UP:
			if (cursor_pos > 0) {
				cursor_pos--;
				redraw_flag = true;
			}
			break;
		case J_DOWN:
			if (cursor_pos < 7) {
				cursor_pos++;
				redraw_flag = true;
			}
			break;
		}

		// Rendering.
		if (cursor_pos * 8 + 20 - cursor_spr > 0)
			cursor_spr += CURSOR_SPEED;
		else if (cursor_pos * 8 + 20 - cursor_spr < 0)
			cursor_spr -= CURSOR_SPEED;
		// Redraw item description if needed.
		if (redraw_flag) {
			draw_description(base_item + cursor_pos);
			redraw_flag = false;
		}
		shadow_OAM[0].y = cursor_spr;
		shadow_OAM[1].y = cursor_spr;
		wait_vbl_done();
	}

	exit:

	shadow_OAM[0].x = 172;
	shadow_OAM[1].x = 180;
	shadow_OAM[0].y = 20 + 16;
	shadow_OAM[1].y = 20 + 16;
	while (SCX_REG > 0) {
		wait_vbl_done();
		SCX_REG -= SUBMENU_SLIDE_SPEED;
		shadow_OAM[0].x += SUBMENU_SLIDE_SPEED;
		shadow_OAM[1].x += SUBMENU_SLIDE_SPEED;
	}
	SCX_REG = 0;

	return 0;
}

u8 pause_menu() BANKED
{
	// Init.
	u8 cursor_pos = 0;
	u8 cursor_spr = 20;

	vmemset((void *)(0x9C00), 0x8E, 18 * 32);

	win_pos.y = 8;
	while (win_pos.x >= 7) {
		render_entities();
		wait_vbl_done();
		win_pos.x -= SWIPE_SPEED;
	}
	win_pos.x = 7;

	while (status_position > 1) {
		win_pos.y--;
		status_position--;
		text_position -= 4;
		wait_vbl_done();
	}

	fx_mode = NO_UI;
	wait_vbl_done();
	lcdc_buffer = \
		LCDC_ENABLE | LCDC_BG_ENABLE | LCDC_BG_SCRN1 | LCDC_OBJ_ENABLE \
		| LCDC_OBJ_16;
	if (_cpu == CGB_TYPE) {
		BCPS_REG = 0x80 | 56;
		BCPD_REG = current_ui_pal.colors[0] & 0xFF;
		BCPD_REG = (current_ui_pal.colors[0] & 0xFF00) >> 8;
	}
	SCX_REG = 0;
	SCY_REG = 0;

	// Draw pause screen.
	set_bkg_1bit_data(0x00, 0x3E, paw_print, 1);
	set_bkg_tiles(9, 8, 11, 10, paw_print_map);
	vmemcpy((void *)(0x8000), SIZE(paw_cursor), paw_cursor);
	vwf_activate_font(0);
	vwf_draw_text(3, 1, FONT_TILE, pause_text);

	shadow_OAM[0].x = 12;
	shadow_OAM[1].x = 20;
	shadow_OAM[0].tile = CURSOR_TILE;
	shadow_OAM[1].tile = CURSOR_TILE + 2;
	shadow_OAM[0].prop = OAM_DMG_PAL1;
	shadow_OAM[1].prop = OAM_DMG_PAL1;
	while(1) {
		update_input();
		if (new_keys & J_UP) {
			if (cursor_pos > 0)
				cursor_pos--;
		} else if (new_keys & J_DOWN) {
			if (cursor_pos < NB_OPTIONS)
				cursor_pos++;
		}
		if (cursor_pos * 16 + 20 - cursor_spr > 0)
			cursor_spr += CURSOR_SPEED;
		else if (cursor_pos * 16 + 20 - cursor_spr < 0)
			cursor_spr -= CURSOR_SPEED;

		if (cur_keys == J_A) {
			switch (cursor_pos) {
			case RETURN_CHOICE:
				goto exit;
			case ITEMS_CHOICE:
				item_menu();
				break;
			//case PARTY_CHOICE:
			//	break;
			//case SAVE_CHOICE:
			//	break;
			//case OPTIONS_CHOICE:
			//	break;
			//case ESCAPE_CHOICE:
			//	break;
			};
		} else if (new_keys == J_START || new_keys == J_B)
			goto exit;

		shadow_OAM[0].y = cursor_spr;
		shadow_OAM[1].y = cursor_spr;

		wait_vbl_done();
	}

	exit:
	memset(shadow_OAM, 0, 8);
	wait_vbl_done();
	for (u8 i = 0; i < NB_ENTITIES; i++) {
		if (entities[i].data) {
			reload_entity_graphics(i);
		}
	}
	vmemset((void *)(0x9C00), 0x8E, 18 * 32);
	vmemset((void *)(0x9FA0), 0x8E, 3 * 32);
	reload_mapdata();
	load_item_graphics();
	fx_mode = GAME_UI;
	wait_vbl_done();
	LCDC_REG = lcdc_buffer = \
		LCDC_ENABLE | LCDC_BG_ENABLE | LCDC_WINDOW_ENABLE | \
		LCDC_WINDOW_SCRN1 | LCDC_OBJ_ENABLE | LCDC_OBJ_16;

	while (status_position != 8) {
		win_pos.y++;
		status_position++;
		text_position += 4;
		wait_vbl_done();
	}

	while (win_pos.x < 160) {
		render_entities();
		wait_vbl_done();
		win_pos.x += SWIPE_SPEED;
	}
	win_pos.x = 160;
	win_pos.y = 72;
	init_hud();

	return 0;
}
