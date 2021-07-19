#pragma bank 255

#include <gb/cgb.h>
#include <string.h>

#include "include/dir.h"
#include "include/entity.h"
#include "include/hud.h"
#include "include/game.h"
#include "include/int.h"
#include "include/item.h"
#include "include/rendering.h"
#include "libs/vwf.h"
#include "menus/party.h"
#include "menus/pause.h"

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
			draw_party_entity(t, DIR_LEFT, IDLE_FRAME);
			for (u8 i = 0; i < 15; i++)
				wait_vbl_done();
			draw_party_entity(t, DIR_LEFT, HURT_FRAME);
			for (u8 i = 0; i < 8; i++)
				wait_vbl_done();
			draw_party_entity(t, DIR_LEFT, ATTACK_FRAME);
			for (u8 i = 0; i < 8; i++)
				wait_vbl_done();
			draw_party_entity(t, DIR_LEFT, IDLE_FRAME);
			for (u8 i = 0; i < 15; i++)
				wait_vbl_done();
			shadow_OAM[0].y = 0;
			shadow_OAM[1].y = 0;
			draw_party(22, 18, PARTYFONT_TILE, 7u * 8u + 8u,
				   9u * 8u + 16u, 24, PARTY_HEALTH);
			for (u8 i = 0; i < 15; i++)
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
	memset(shadow_OAM, 0, 160);
	return true;
}

void draw_inventory(u8 start) NONBANKED
{
	u8 temp_bank = _current_bank;

	for (u8 i = 0; i < 16; i++)
		vmemset((void *)(0x9C35 + i * 32), BLANK_TILE, 10);
	for (u8 i = 0; (start < INVENTORY_SIZE) && (i < 8); i++, start++) {
		if (inventory[start].data) {
			SWITCH_ROM_MBC1(inventory[start].bank);
			vwf_draw_text(24, 1 + i, SUBFONT_TILE + i * 8, 
				      inventory[start].data->name);
		}
	}

	SWITCH_ROM_MBC1(temp_bank);
}

void draw_item_cursor(u8 i) NONBANKED
{
	u8 temp_bank = _current_bank;
	SWITCH_ROM_MBC1(inventory[i].bank);
	vmemcpy((void *)(0x8000), 16, inventory[i].data->graphic);
	vmemcpy((void *)(0x8010), 16, &inventory[i].data->graphic[32]);
	vmemcpy((void *)(0x8020), 16, &inventory[i].data->graphic[16]);
	vmemcpy((void *)(0x8030), 16, &inventory[i].data->graphic[48]);
	if (_cpu == CGB_TYPE)
		set_sprite_palette(0, 1, inventory[i].data->colors);
	SWITCH_ROM_MBC1(temp_bank);
}

bool use_item_menu(u8 base_item, u8 i) BANKED
{
	u8 cursor_pos = 0;
	u8 cursor_spr = 88;

	// Init
	// Copy the item sprite to VRAM
	draw_party(22, 18, PARTYFONT_TILE, 7u * 8u + 8u, 18u * 8u + 16u, 24,
		PARTY_HEALTH);
	shadow_OAM[0].x = 6u * 8u;
	shadow_OAM[1].x = 6u * 8u + 8u;
	shadow_OAM[0].y = 88u + SUBSUBMENU_SLIDE_POS;
	shadow_OAM[1].y = 88u + SUBSUBMENU_SLIDE_POS;
	draw_item_cursor(i);
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
				return true;
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
	shadow_OAM[0].x = 10u * 8u + 4u;
	shadow_OAM[1].x = 10u * 8u + 12u;
	vmemcpy((void *)(0x8000), SIZE(paw_cursor), paw_cursor);
	set_sprite_palette(0, 1, current_ui_pal.colors);
	return false;
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

bool item_menu() BANKED
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
			if (cursor_pos < 8 &&
			    cursor_pos + base_item < INVENTORY_SIZE &&
			    inventory[base_item + cursor_pos + 1].data) {
				cursor_pos++;
				redraw_flag = true;
			}
			break;
		case J_A:
			if (inventory[base_item + cursor_pos].data)
				if (use_item_menu(base_item, base_item + cursor_pos))
					return true;
			if (!inventory[base_item + cursor_pos + 1].data &&
			    cursor_pos > 0) {
				cursor_pos--;
				redraw_flag = true;
				}
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

	return false;
}