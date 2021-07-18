#pragma bank 255

#include <gb/cgb.h>
#include <stdbool.h>
#include <stdio.h>
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

// VRAM allocation
#define CURSOR_TILE 0x00u
#define CURSOR_PALETTE 0u
#define ENTITY_TILE 0x04u
#define ENTITY_PALETTE 1u
#define BLANK_TILE 0x8Eu
#define FONT_TILE 0x8Fu
#define SUBFONT_TILE 0xA6u
#define PARTYFONT_TILE 0x35u
#define DESCFONT_TILE (SUBFONT_TILE + 56u)

// Animation speeds
#define CURSOR_SPEED 4u
#define SWIPE_SPEED 12u
#define SUBMENU_SLIDE_SPEED 8u

#define SUBMENU_SLIDE_POS 0x60u
#define SUBSUBMENU_SLIDE_POS 0x48u
#define NB_OPTIONS (6u - 1u)

enum choice {
	RETURN_CHOICE,
	ITEMS_CHOICE,
	PARTY_CHOICE,
	SAVE_CHOICE,
	OPTIONS_CHOICE,
	ESCAPE_CHOICE
};

enum party_type {
	PARTY_NULL = 0,
	PARTY_HEALTH = 1,
	PARTY_FATIGUE = 2,
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
const char health_text[] = \
"HP: %u/%u";

// Banked portion of draw_party.
void draw_party_banked(u8 i) NONBANKED
{
	u8 temp_bank = _current_bank;
	SWITCH_ROM_MBC1(entities[i].bank);
	vmemcpy((void *)(0x8040 + i * 64), 64,
		&entities[i].data->graphics[DIR_DOWN * NB_UNIQUE_TILES * 16]);
	if (_cpu == CGB_TYPE)
		set_sprite_palette(i + 1, 1, entities[i].data->colors);
	SWITCH_ROM_MBC1(temp_bank);
}

/**
 * Draws the party menu starting at a given tile. Can be used to show the party
 * or select who to use an item on.
 * 
 * @param x		Starting tile position of the menu.
 * @param y
 * @param font_tile	The VRAM tile to use as font space. This allows text to
 * be overwritten without conflicts when using the party screen as a submenu.
 * @param spr_x		The starting position of the party sprites. Used to
 * control where they appear before scrolling.
*/
void draw_party(u8 x, u8 y, u8 font_tile, u8 spr_x, u8 spr_y, u8 spacing, 
	u8 type) BANKED
{
	char buffer[16];
	vwf_draw_text(0, 0, font_tile, "\0");
	for (u8 i = 0; i < NB_ALLIES; i++) {
		if (entities[i].data) {
			vwf_draw_text(x, y++, vwf_next_tile(),
				      entities[i].name);
			if (type & PARTY_HEALTH) {
				sprintf(buffer, health_text, entities[i].health,
					entities[i].max_health);
				vwf_draw_text(x, y++, vwf_next_tile(), buffer);
			}
			if (type & PARTY_FATIGUE) {
				vwf_draw_text(x, y++, vwf_next_tile(),
					      "Fatigue: 100/100");
			}
			char *entry = (char *)&shadow_OAM[2 + i * 2];
			*entry++ = spr_y + i * spacing;
			*entry++ = spr_x;
			*entry++ = ENTITY_TILE + i * 4;
			*entry++ = ENTITY_PALETTE + i;
			*entry++ = spr_y + i * spacing;
			*entry++ = spr_x + 8;
			*entry++ = ENTITY_TILE + i * 4 + 2;
			*entry++ = ENTITY_PALETTE + i;
			draw_party_banked(i);
			y++;
		}
	}
}

bool use_item(u8 i, u8 t)
{
	entity *target = &entities[t];
	item *src_item = &inventory[i];
	switch (src_item->data->type) {
	case HEAL_ITEM:
		if (target->health < target->max_health) {
			if (target->health + ((healitem_data *)src_item->data)->health >= target->max_health)
				target->health = target->max_health;
			else
				target->health += ((healitem_data *)src_item->data)->health;
			draw_party(22, 18, PARTYFONT_TILE, 7u * 8u + 8u,
				   9u * 8u + 16u, 24, PARTY_HEALTH);
			for (u8 i = 0; i < 60; i++)
				wait_vbl_done();
			goto consume_item;
		}
		else
			return false;
		break;
	default:
		return false;
	}

	consume_item:
	// Move the inventory down by one index and clear the last slot.
	memmove(src_item, &inventory[i + 1], (INVENTORY_SIZE - 1u - i) * sizeof(item));
	memset(&inventory[INVENTORY_SIZE - 1], 0, sizeof(item));
	return true;
}

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

void use_item_menu(u8 base_item, u8 i) BANKED
{
	u8 cursor_pos = 0;
	u8 cursor_spr = 88;

	// Init
	draw_party(22, 18, PARTYFONT_TILE, 7u * 8u + 8u,
		   18u * 8u + 16u, 24, PARTY_HEALTH);
	shadow_OAM[0].x = 6u * 8u;
	shadow_OAM[1].x = 6u * 8u + 8u;
	shadow_OAM[0].y = 88u + SUBSUBMENU_SLIDE_POS;
	shadow_OAM[1].y = 88u + SUBSUBMENU_SLIDE_POS;
	while (SCY_REG < SUBSUBMENU_SLIDE_POS) {
		wait_vbl_done();
		SCY_REG += SUBMENU_SLIDE_SPEED;
		for (u8 i = 0; i < 8; i++) {
			if (!shadow_OAM[i].y)
				continue;
			shadow_OAM[i].y -= SUBMENU_SLIDE_SPEED;
		}
	}
	SCY_REG = SUBSUBMENU_SLIDE_POS;

	while (1) {
		// Handle new inputs.
		update_input();
		switch (new_keys) {
		case J_START: case J_B:
			goto exit;
		case J_UP:
			if (cursor_pos > 0) {
				cursor_pos--;
			}
			break;
		case J_DOWN:
			if (cursor_pos < 2) {
				cursor_pos++;
			}
			break;
		case J_A:
			if (use_item(i, cursor_pos))
				goto exit;
			break;
		}

		// Rendering.
		if (cursor_pos * 24 + 88 - cursor_spr > 0)
			cursor_spr += CURSOR_SPEED;
		else if (cursor_pos * 24 + 88 - cursor_spr < 0)
			cursor_spr -= CURSOR_SPEED;
		shadow_OAM[0].y = cursor_spr;
		shadow_OAM[1].y = cursor_spr;
		wait_vbl_done();
	}

	exit:
	draw_inventory(base_item);

	while (SCY_REG > 0) {
		wait_vbl_done();
		SCY_REG -= SUBMENU_SLIDE_SPEED;
		for (u8 i = 0; i < 8; i++) {
			if (!shadow_OAM[i].y)
				continue;
			shadow_OAM[i].y += SUBMENU_SLIDE_SPEED;
		}
	}
	SCY_REG = 0;
}

void draw_description(u8 i) NONBANKED
{
	if (!inventory[i].data) {
		for (u8 y = 0; y < 4; y++)
			vmemset((void *)(0x9DB5 + y * 32), BLANK_TILE, 10);
		return;
	}
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
		case J_A:
			use_item_menu(base_item, base_item + cursor_pos);
			redraw_flag = true;
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
