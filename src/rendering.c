#pragma bank 255

#include <gb/gb.h>
#include <stdbool.h>
#include <stdint.h>

#include "include/dialogue.h"
#include "include/entity.h"
#include "include/game.h"
#include "include/hardware.h"
#include "include/hud.h"
#include "include/rendering.h"
#include "include/vec.h"

#define SWIPE_SPEED 12u

uint8_t lcdc_buffer;
uint8_t oam_index = 0;
uvec8 scr_pos = {0, 0};
uvec8 win_pos = {168, 72};
uint8_t fx_mode = NO_UI;

void vblank() NONBANKED
{
	switch(fx_mode) {
	case DIALOGUE_UI:
		LYC_REG = 144 - 32;
		add_LCD(&show_dialogue);
	case NO_UI:
		SCX_REG = scr_pos.x;
		SCY_REG = scr_pos.y;
		LCDC_REG = lcdc_buffer;
		break;
	case GAME_UI:
		show_hud();
		break;
	}
	WX_REG = win_pos.x;
	WY_REG = win_pos.y;
	update_input();
}

// TODO: This might be broken? I had some issue with it when doing the
// inventory. Might have just been a mistake.
void vmemcpy(void *dest, uint16_t len, const void *src) NONBANKED
{
	dest; len; src;
	__asm
		// Init Args
		ldhl sp, #2
		ld a, (hl+)
		ld e, a
		ld a, (hl+)
		ld d, a
		ld a, (hl+)
		ld c, a
		ld a, (hl+)
		ld b, a
		ld a, (hl+)
		ld h, (hl)
		ld l, a
		dec bc
		inc b
		inc c
	00001$:
		ldh a, (_STAT_REG)
		and a, #2
		jr nz, 00001$
		ld a, (hl+)
		ld (de), a
		inc de
		dec c
		jr nz, 00001$
		dec b
		jr nz, 00001$
	__endasm;
}

void banked_vmemcpy(void *dest, uint16_t len, const void *src, uint8_t bank) NONBANKED
{
	uint8_t temp_bank = _current_bank;
	SWITCH_ROM_MBC1(bank);
	vmemcpy(dest, len, src);
	SWITCH_ROM_MBC1(temp_bank);
}

void vsetmap(uint8_t *vram_addr, uint8_t w, uint8_t h, const uint8_t *tiles) NONBANKED
{
	for (; h; h--) {
		vmemcpy(vram_addr, w, tiles);
		vram_addr += 0x20;
		tiles += w;
	}
}

void banked_vsetmap(uint8_t *vram_addr, uint8_t w, uint8_t h, const uint8_t *tiles,
		    uint8_t bank) NONBANKED
{
	uint8_t temp_bank = _current_bank;
	SWITCH_ROM_MBC1(bank);
	vsetmap(vram_addr, w, h, tiles);
	SWITCH_ROM_MBC1(temp_bank);
}

void reset_oam() BANKED
{
	//uint8_t *oam_ptr = (uint8_t *) shadow_OAM;
	//while(oam_ptr != (shadow_OAM + 160) {
	//	*oam_ptr = 0;
	//	oam_ptr += 4;
	//}

	__asm
		ld	hl, #_shadow_OAM
	2$:
		ld	a, l
		cp	a, #160
		ret	Z
	
		xor	a, a
		ld	(hl+), a
		inc	l
		inc	l
		inc	l
		jr	2$
	__endasm;
}

/**
 * Cleans leftover oam entries so that unused sprites are not rendered. Resets
 * oam_index, allowing a new set of sprites to be rendered. This function should
 * be run after rendering.
*/
void clean_oam() BANKED
{
	static uint8_t last_oam = 0;

	uint8_t tmp = oam_index;
	while(oam_index < last_oam) {
		shadow_OAM[oam_index++].y = 0;
	}
	last_oam = tmp;
	oam_index = 0;
}

void fade_to_white(uint8_t fade_speed) BANKED
{
	if (_cpu == CGB_TYPE) {
		while(1) {
			uint8_t completion = 0;
			for (uint8_t i = 0; i < 8 * 7;) {
				BCPS_REG = i++;
				WAIT_VRAM;
				short cur_pal = BCPD_REG;
				BCPS_REG = i++;
				WAIT_VRAM;
				cur_pal |= BCPD_REG << 8;
				uint8_t color[3] = {
					cur_pal & 0b11111,
					(cur_pal & 0b1111100000) >> 5,
					(cur_pal & 0b111110000000000) >> 10
				};
				for (uint8_t j = 0; j < 3; j++) {
					if (color[j] + fade_speed < 31) {
						color[j] += fade_speed;
					} else {
						color[j] = 31;
						completion++;
					}
				}
				cur_pal = color[0] | (color[1] << 5) | (color[2] << 10);
				BCPS_REG = 0x80 | (i - 2);
				WAIT_VRAM;
				BCPD_REG = cur_pal;
				BCPD_REG = cur_pal >> 8;
			}
			for (uint8_t i = 0; i < 8 * 7;) {
				OCPS_REG = i++;
				WAIT_VRAM;
				short cur_pal = OCPD_REG;
				OCPS_REG = i++;
				WAIT_VRAM;
				cur_pal |= OCPD_REG << 8;
				uint8_t color[3] = {
					cur_pal & 0b11111,
					(cur_pal & 0b1111100000) >> 5,
					(cur_pal & 0b111110000000000) >> 10
				};
				for (uint8_t j = 0; j < 3; j++) {
					if (color[j] + fade_speed < 31) {
						color[j] += fade_speed;
					} else {
						color[j] = 31;
						completion++;
					}
				}
				cur_pal = color[0] | (color[1] << 5) | (color[2] << 10);
				OCPS_REG = 0x80 | (i - 2);
				WAIT_VRAM;
				OCPD_REG = cur_pal;
				OCPD_REG = cur_pal >> 8;
			}
			if (completion >= 8 * 7 * 2)
				return;
		}
	} else {
		for (uint8_t i = 0; i < 4; i++) {
			wait_vbl_done();
			wait_vbl_done();
			BGP_REG <<= 2;
			OBP0_REG <<= 2;
			OBP1_REG <<= 2;
		}
	}
}

void swipe_left(bool render) BANKED
{
	vmemset((char*) (0x9C00), 0x8E, 27 * 32);

	win_pos.y = 7;
	while (win_pos.x >= 7) {
		if (render)
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
}

void swipe_right() BANKED
{
	vmemset((char*) (0x9FA0), 0x8E, 3 * 32);

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
}