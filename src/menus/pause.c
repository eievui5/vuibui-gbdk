#pragma bank 255

#include <gb/cgb.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "include/bank.h"
#include "include/dir.h"
#include "include/entity.h"
#include "include/hud.h"
#include "include/game.h"
#include "include/hardware.h"
#include "include/item.h"
#include "include/map.h"
#include "include/rendering.h"
#include "libs/vwf.h"
#include "menus/inventory.h"
#include "menus/options.h"
#include "menus/party.h"
#include "menus/pause.h"

enum choice {
	RETURN_CHOICE,
	ITEMS_CHOICE,
	PARTY_CHOICE,
	SAVE_CHOICE,
	OPTIONS_CHOICE,
	ESCAPE_CHOICE
};

INCBIN(paw_print, res/gfx/ui/paw_mark.1bpp)
INCBIN_EXTERN(paw_print)
INCBIN(paw_print_map, res/gfx/ui/paw_mark.map)
INCBIN_EXTERN(paw_print_map)
INCBIN(paw_cursor, res/gfx/ui/paw_cursor.h.2bpp)
DEF_BANK(pause_menu)

const char pause_text[] = \
"Return\n\nItems\n\nParty\n\nSave\n\nOptions\n\nEscape!";

// Scroll the screen and OAM entires to the right.
void slide_into_submenu_x() BANKED
{
	while (SCX_REG < SUBMENU_SLIDE_POS) {
		wait_vbl_done();
		SCX_REG += SUBMENU_SLIDE_SPEED;
		for (uint8_t i = 0; i < 8; i++) {
			if (!shadow_OAM[i].y)
				continue;
			shadow_OAM[i].x -= SUBMENU_SLIDE_SPEED;
		}
	}
	SCX_REG = SUBMENU_SLIDE_POS;
}

// Scroll the screen and OAM entires to the left.
void slide_out_submenu_x() BANKED
{
	while (SCX_REG > 0) {
		wait_vbl_done();
		SCX_REG -= SUBMENU_SLIDE_SPEED;
		for (uint8_t i = 0; i < 8; i++) {
			if (!shadow_OAM[i].y)
				continue;
			shadow_OAM[i].x += SUBMENU_SLIDE_SPEED;
		}
	}
	SCX_REG = 0;
}

// Show and handle the pause menu. Return true if a turn has been taken.
bool pause_menu() BANKED
{
	// Init.
	uint8_t cursor_pos = 0;
	uint8_t cursor_spr = 20;
	bool used_turn = false;

	swipe_left(true);
	lcdc_buffer = \
		LCDC_ENABLE | LCDC_BG_ENABLE | LCDC_BG_SCRN1 | LCDC_OBJ_ENABLE \
		| LCDC_OBJ_16;

	// Draw pause screen.
	set_bkg_1bit_data(0x00, 0x3E, paw_print, 1);
	vsetmap((void *) 0x9D09, 11, 10, paw_print_map);
	vmemcpy((void *)(0x8000), SIZE(paw_cursor), paw_cursor);
	set_sprite_palette(0, 1, current_ui_pal.colors);
	vwf_activate_font(0);
	vwf_draw_text(3, 1, FONT_TILE, pause_text);

	shadow_OAM[0].x = 12;
	shadow_OAM[1].x = 20;
	shadow_OAM[0].tile = CURSOR_TILE;
	shadow_OAM[1].tile = CURSOR_TILE + 2;
	shadow_OAM[0].prop = OAM_DMG_PAL1;
	shadow_OAM[1].prop = OAM_DMG_PAL1;
	while(1) {
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
				if (item_menu()) {
					used_turn = true;
					goto exit;
				}
				break;
			case PARTY_CHOICE:
				party_menu();
				break;
			//case SAVE_CHOICE:
			//	break;
			case OPTIONS_CHOICE:
				options_menu();
				break;
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
	for (uint8_t i = 0; i < NB_ENTITIES; i++) {
		if (entities[i].data) {
			reload_entity_graphics(i);
		}
	}
	vmemset((void *)(0x9C00), 0x8E, 27 * 32);
	reload_mapdata();
	load_item_graphics();

	swipe_right();

	return used_turn;
}
