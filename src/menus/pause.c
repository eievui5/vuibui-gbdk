#pragma bank 255

#include <gb/cgb.h>
#include <stdbool.h>
#include <string.h>

#include "include/bank.h"
#include "include/dir.h"
#include "include/entity.h"
#include "include/hud.h"
#include "include/game.h"
#include "include/hardware.h"
#include "include/int.h"
#include "include/item.h"
#include "include/map.h"
#include "include/rendering.h"
#include "libs/vwf.h"
#include "menus/inventory.h"
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

INCBIN(paw_print, res/gfx/ui/paw_mark.2bpp)
INCBIN_EXTERN(paw_print)
INCBIN(paw_print_map, res/gfx/ui/paw_mark.map)
INCBIN_EXTERN(paw_print_map)
INCBIN(paw_cursor, res/gfx/ui/paw_cursor.h.2bpp)
DEF_BANK(pause_menu)

const char pause_text[] = \
"Return\n\nItems\n\nParty\n\nSave\n\nOptions\n\nEscape!";

// Show and handle the pause menu. Return true if a turn has been taken.
bool pause_menu() BANKED
{
	// Init.
	u8 cursor_pos = 0;
	u8 cursor_spr = 20;
	bool used_turn = false;

	vmemset((void *)(0x9C00), 0x8E, 27 * 32);

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
	vmemset((void *)(0x9C00), 0x8E, 27 * 32);
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

	while (win_pos.x < 168) {
		render_entities();
		wait_vbl_done();
		win_pos.x += SWIPE_SPEED;
	}
	win_pos.x = 168;
	win_pos.y = 72;
	init_hud();

	return used_turn;
}
