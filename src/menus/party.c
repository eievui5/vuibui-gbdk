#pragma bank 255

#include <gb/cgb.h>
#include <stdio.h>
#include <string.h>

#include "include/dir.h"
#include "include/entity.h"
#include "include/game.h"
#include "include/int.h"
#include "include/rendering.h"
#include "libs/vwf.h"
#include "menus/party.h"
#include "menus/pause.h"

#define ENTITY_TILE 0x04u
#define ENTITY_PALETTE 1u

const char health_text[] = "HP: %u/%u";
const char hunger_text[] = "Fatigue: %u/%u";

// Draws an entity with a static frame and direction.
void draw_party_entity(u8 i, u8 dir, u8 frame) NONBANKED
{
	u8 temp_bank = _current_bank;
	SWITCH_ROM_MBC1(entities[i].bank);
	vmemcpy((void *)(0x8040 + i * 64), 64,
		&entities[i].data->graphics[
			dir * NB_UNIQUE_TILES  * 16 + 64 * frame]
	);
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
 * @param spr_y
 * @param spacing	Distance between each entry.
 * @param type		The info to display.
*/
void draw_party(u8 x, u8 y, u8 font_tile, u8 spr_x, u8 spr_y, u8 spacing, 
	u8 type) BANKED
{
	char buffer[16];
	vwf_draw_text(0, 0, font_tile, "");
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
			draw_party_entity(i, DIR_DOWN, IDLE_FRAME);
			y++;
		}
	}
}

void party_menu() BANKED
{
	u8 cursor_pos = 0;
	u8 cursor_spr = 20;

	for (u8 i = 0; i < 16; i++)
		vmemset((void *)(0x9C35 + i * 32), BLANK_TILE, 10);
	draw_party(22, 1, PARTYFONT_TILE, 20u * 8u + 8u, 8u + 16u, 32,
		PARTY_HEALTH | PARTY_FATIGUE);
	shadow_OAM[0].x = 19u * 8u;
	shadow_OAM[1].x = 19u * 8u + 8u;
	shadow_OAM[0].y = 24;
	shadow_OAM[1].y = 24;
	while (SCX_REG < SUBMENU_SLIDE_POS) {
		wait_vbl_done();
		SCX_REG += SUBMENU_SLIDE_SPEED;
		for (u8 i = 0; i < 8; i++) {
			if (!shadow_OAM[i].y)
				continue;
			shadow_OAM[i].x -= SUBMENU_SLIDE_SPEED;
		}
	}
	SCX_REG = SUBMENU_SLIDE_POS;

	while (1) {
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
		}
		if (cursor_pos * 32 + 24 - cursor_spr > 0)
			cursor_spr += CURSOR_SPEED;
		else if (cursor_pos * 32 + 24 - cursor_spr < 0)
			cursor_spr -= CURSOR_SPEED;
		shadow_OAM[0].y = cursor_spr;
		shadow_OAM[1].y = cursor_spr;
		wait_vbl_done();
	}

	exit:
	shadow_OAM[0].x = 172;
	shadow_OAM[1].x = 180;
	shadow_OAM[0].y = 20 + 32;
	shadow_OAM[1].y = 20 + 32;
	while (SCX_REG > 0) {
		wait_vbl_done();
		SCX_REG -= SUBMENU_SLIDE_SPEED;
		for (u8 i = 0; i < 8; i++) {
			if (!shadow_OAM[i].y)
				continue;
			shadow_OAM[i].x += SUBMENU_SLIDE_SPEED;
		}
	}
	SCX_REG = 0;
	memset(&shadow_OAM[2], 0, 3 * 4);
}