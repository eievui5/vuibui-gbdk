#include <gb/cgb.h>
#include <stdint.h>
#include <string.h>

#include "include/entity.h"
#include "include/game.h"
#include "include/hardware.h"
#include "include/item.h"
#include "include/rendering.h"
#include "include/save.h"

#include "entities/aris.h"
#include "entities/forest_rat.h"
#include "entities/luvui.h"

void main()
{
	if (_cpu == CGB_TYPE)
		cpu_fast();
	wait_vbl_done();
	LCDC_REG = 0;
	add_VBL(&vblank);
	set_interrupts(VBL_IFLAG | LCD_IFLAG);
	STAT_REG = STAT_LYC;
	BGP_REG = 0b11100100;
	OBP0_REG = 0b11010000;
	OBP1_REG = 0b11100100;

	test_sram_corruption();

	memset(entities, 0, sizeof(entities));
	memset(world_items, 0, sizeof(world_items));
	memset(inventory, 0, sizeof(inventory));
	memset(&active_save_file, 0, sizeof(save_file));
	new_entity(&luvui_entity, BANK(luvui), 0, 32, 32, 5);
	new_entity(&aris_entity, BANK(aris), 1, 33, 32, 5);
	new_entity(&forest_rat_entity, BANK(forest_rat), 2, 32, 33, 5);

	LCDC_REG = lcdc_buffer = LCDC_ENABLE;
	game_state = TITLESCREEN_STATE;
	game_loop();
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