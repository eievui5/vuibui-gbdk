#pragma bank 255

#include <gb/cgb.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "include/dir.h"
#include "include/entity.h"
#include "include/game.h"
#include "include/rendering.h"
#include "libs/vwf.h"
#include "menus/party.h"
#include "menus/pause.h"

#define ENTITY_TILE 0x04u
#define ENTITY_PALETTE 1u

const char level_text[] = "Lvl: %u";
const char health_text[] = "Hp: %u/%u";
const char hunger_text[] = "Ftg: %u/%u";

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
void draw_party(uint8_t x, uint8_t y, uint8_t font_tile, uint8_t spr_x, 
	uint8_t spr_y, uint8_t spacing, uint8_t type) BANKED
{
	char buffer[16];
	vwf_draw_text(0, 0, (char*) 0x9C00, font_tile, "");
	for (uint8_t i = 0; i < NB_ALLIES; i++) {
		if (entities[i].data) {
			vwf_draw_text(x, y++, (char*) 0x9C00, vwf_next_tile(),
				      entities[i].name);
			if (type & PARTY_LEVEL) {
				sprintf(buffer, level_text, entities[i].level);
				vwf_draw_text(x, y++, (char*) 0x9C00,
					      vwf_next_tile(), buffer);
			}
			if (type & PARTY_HEALTH) {
				sprintf(buffer, health_text, entities[i].health,
					get_max_health(&entities[i]));
				vwf_draw_text(x, y++, (char*) 0x9C00,
					      vwf_next_tile(), buffer);
			}
			if (type & PARTY_FATIGUE) {
				sprintf(buffer, hunger_text,
					entities[i].fatigue >> 4u,
					get_max_fatigue(&entities[i]) >> 4u);
				vwf_draw_text(x, y++, (char*) 0x9C00,
					      vwf_next_tile(), buffer);
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
			draw_static_entity(&entities[i], DIR_DOWN, IDLE_FRAME,
					   (void *) (0x8040 + i * 64), i + 1);
			y++;
		}
	}
}

void party_menu() BANKED
{
	uint8_t cursor_pos = 0;
	uint8_t cursor_spr = 20;

	for (uint8_t i = 0; i < 16; i++)
		vmemset((void *)(0x9C35 + i * 32), BLANK_TILE, 10);
	draw_party(22, 1, PARTYFONT_TILE, 20u * 8u + 8u, 8u + 16u, 40,
		PARTY_LEVEL | PARTY_HEALTH | PARTY_FATIGUE);
	shadow_OAM[0].x = 19u * 8u;
	shadow_OAM[1].x = 19u * 8u + 8u;
	shadow_OAM[0].y = 24;
	shadow_OAM[1].y = 24;
	slide_into_submenu_x();

	while (1) {
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
		if (cursor_pos * 40 + 24 - cursor_spr > 0)
			cursor_spr += CURSOR_SPEED;
		else if (cursor_pos * 40 + 24 - cursor_spr < 0)
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
	slide_out_submenu_x();
	memset(&shadow_OAM[2], 0, 6 * 4);
}