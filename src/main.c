#include <gb/cgb.h>
#include <gb/gb.h>
#include <rand.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "include/entity.h"
#include "include/game.h"
#include "include/hardware.h"
#include "include/hud.h"
#include "include/item.h"
#include "include/map.h"
#include "include/rendering.h"
#include "include/vec.h"
#include "include/world.h"
#include "libs/vwf.h"
#include "menus/title.h"

#include "entities/luvui.h"
#include "items/apple.h"
#include "mapdata/debug_mapdata.h"
#include "moves/lunge.h"

void main()
{
	if (_cpu == CGB_TYPE)
		cpu_fast();
	wait_vbl_done();
	LCDC_REG = 0;
	add_VBL(&vblank);
	fx_mode = NO_UI;
	set_interrupts(VBL_IFLAG | LCD_IFLAG);
	STAT_REG = STAT_LYC;
	BGP_REG = 0b11100100;
	OBP0_REG = 0b11010000;
	OBP1_REG = 0b11100100;


	memset(entities, 0, sizeof(entities));
	memset(world_items, 0, sizeof(world_items));
	memset(inventory, 0, sizeof(inventory));

	LCDC_REG = lcdc_buffer = \
		LCDC_ENABLE | LCDC_BG_ENABLE | LCDC_BG_SCRN1 | \
		LCDC_OBJ_ENABLE | LCDC_OBJ_16;

	show_title();
	simulate_worldmap();
	memset(shadow_OAM, 0, 160);

	init_hud(); 
	initrand(DIV_REG);

	//current_mapdata = &debug_mapdata;
	//current_mapdata_bank = BANK(debug_mapdata);
	reload_mapdata();
	swipe_left(false);
	new_entity(&luvui_entity, BANK(luvui), 0, 32, 32, 5);
	new_entity(&luvui_entity, BANK(luvui), 1, 33, 32, 5);
	new_entity(&luvui_entity, BANK(luvui), 2, 32, 33, 5);
	strcpy(PLAYER.name, "Eievui");
	create_new_floor();
	swipe_right();
	simulate_gameplay();
}

// Trampoline for accessing banked ROM.
uint8_t banked_get(uint8_t bank, const uint8_t *value) __naked
{
	//uint8_t temp_bank = _current_bank;
	//SWITCH_ROM_MBC1(bank);
	//uint8_t ret_val = *value;
	//SWITCH_ROM_MBC1(temp_bank);
	//return ret_val;

	value; bank;
	__asm
		ldh a, (__current_bank)
		ld b, a
		ldhl sp, #2
		ld a, (hl+)
		ldh (__current_bank), a
		ld (#0x2000), a
		ld a, (hl+)
		ld h, (hl)
		ld l, a
		ld e, (hl)
		ld a, b
		ldh (__current_bank), a
		ld (#0x2000), a
		ret
	__endasm;
}