#pragma bank 255

#include <gb/gb.h>
#include <gb/cgb.h>
#include <gb/incbin.h>

#include "include/bank.h"
#include "include/cutscene.h"
#include "include/game.h"
#include "include/hud.h"
#include "include/rendering.h"
#include "libs/vwf.h"

#define DIALOGUE_TEXT_START 0x83
#define DIALOGUE_BOX_PTR ((char*) 0x9F80)

INCBIN_EXTERN(worldmap_ui_gfx)

uint8_t dialogue_bank;
const char* dialogue_string;
static uint8_t* ui_dest_ptr;

void show_dialogue() NONBANKED
{
	remove_LCD(&show_dialogue);
	WAIT_VRAM;
	SCX_REG = 0;
	SCY_REG = 14 * 8;
	LCDC_REG = LCDC_ENABLE | LCDC_BG_ENABLE | LCDC_OBJ_16 | LCDC_BG_SCRN1;
}

void render_char() NONBANKED
{
	uint8_t temp_bank = _current_bank;
	SWITCH_ROM_MBC1(dialogue_bank);
	if (vwf_print_render(*dialogue_string)) {
		set_vram_byte(ui_dest_ptr++, vwf_current_tile - 1);
	}
	if (vwf_current_offset) {
		set_vram_byte(ui_dest_ptr, vwf_current_tile);
	}
	SWITCH_ROM_MBC1(temp_bank);
}

#include "worldmaps/crater.h"
extern const char *string;

// Used to display dialogue boxes; does not work during gameplay!
void print_dialogue() BANKED
{
	// Draw text box.
	banked_vmemcpy((char*) 0x8800, SIZE(worldmap_ui_gfx), worldmap_ui_gfx, BANK(worldmap_ui_gfx));
	vmemset((char*) 0x8800 + SIZE(worldmap_ui_gfx), 0, 16 * 32 * 3);
	vmemset((char*) DIALOGUE_BOX_PTR, 0x80, 20);
	vmemset((char*) DIALOGUE_BOX_PTR + 32, 0x81, 32 * 3);
	if (_cpu == CGB_TYPE) {
		set_bkg_palette(7, 1, current_ui_pal.colors);
		VBK_REG = 1;
		vmemset((char*) 0x9F80, 7, 0x74);
		VBK_REG = 0;
	}
	// Set UI mode to display the text box.
	fx_mode = DIALOGUE_UI;

	static uint8_t* ui_dest_base;
	vwf_print_reset(DIALOGUE_TEXT_START);
	ui_dest_ptr = ui_dest_base = DIALOGUE_BOX_PTR + 32;

	while (1) {
		wait_vbl_done();
		wait_vbl_done();
		switch (banked_get(dialogue_bank, dialogue_string)) {
		case '%':
			dialogue_string++;
			switch (banked_get(dialogue_bank, dialogue_string)) {
			case 'c':
				vmemset((char*) 0x8800 + SIZE(worldmap_ui_gfx), 0, 16 * 32 * 3);
				vmemset(DIALOGUE_BOX_PTR + 32, 0x81, 32 * 3);
				vwf_print_reset(DIALOGUE_TEXT_START);
				ui_dest_ptr = ui_dest_base = DIALOGUE_BOX_PTR + 32;
				break;
			case 'q':
				ui_dest_base += 32;
				ui_dest_ptr = ui_dest_base + 3;
				if (vwf_current_offset) {
					vwf_print_reset(vwf_current_tile + 1u);
				}
				while (1) {
					dialogue_string++;
					wait_vbl_done();
					wait_vbl_done();
					switch (banked_get(dialogue_bank, dialogue_string)) {
					case '\n':
						ui_dest_base += 32;
						ui_dest_ptr = ui_dest_base + 3;
						if (vwf_current_offset) {
							vwf_print_reset(vwf_current_tile + 1u);
						}
						break;
					case 0:
						goto allow_response;
					default:
						render_char();
						break;
					}
				}
				allow_response: {
					WAIT_VRAM;
					*(uint8_t*) 0x9FC1 = 0x82;
					*(uint8_t*) 0x9FE1 = 0x81;
					bool choice = false;
					while (1) {
						if (new_keys & J_UP || new_keys & J_DOWN) {
							choice = !choice;
							if (choice) {
								WAIT_VRAM;
								*(uint8_t*) 0x9FC1 = 0x81;
								*(uint8_t*) 0x9FE1 = 0x82;
							} else {
								WAIT_VRAM;
								*(uint8_t*) 0x9FC1 = 0x82;
								*(uint8_t*) 0x9FE1 = 0x81;
							}
						}
						if (new_keys & J_A) {
							script_return = choice;
							goto exit;
						}
						wait_vbl_done();
					}
				}
			case 'w':
				while(1) {
					if (new_keys & J_A)
						break;
					wait_vbl_done();
				}
				break;
			}
			break;
		case 0:
			while(1) {
				if (new_keys & J_A)
					goto exit;
				wait_vbl_done();
			}
			break;
		case '\n':
			ui_dest_ptr = ui_dest_base += 32;
			if (vwf_current_offset) {
				vwf_print_reset(vwf_current_tile + 1u);
			}
			break;
		default:
			render_char();
			break;
		}
		dialogue_string++;
	}

	exit:
	fx_mode = NO_UI;
	wait_vbl_done();
}