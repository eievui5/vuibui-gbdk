#pragma bank 255

#include <gb/gb.h>

#include "include/hardware.h"
#include "include/hud.h"
#include "include/int.h"
#include "include/rendering.h"
#include "include/vec.h"

u8 lcdc_buffer;
u8 oam_index = 0;
uvec8 win_pos = {160, 144 - 72};

void vblank() NONBANKED
{
	show_hud();
	WX_REG = win_pos.x;
	WY_REG = win_pos.y;
}

void vmemcpy(void *dest, u8 len, const void *src) NONBANKED
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
		ld b, a
		ld a, (hl+)
		ld h, (hl)
		ld l, a
	00001$:
		ldh a, (_STAT_REG)
		and a, #2
		jr nz, 00001$
		ld a, (hl+)
		ld (de), a
		inc de
		dec b
		jr nz, 00001$
	__endasm;
}

/**
 * Cleans leftover oam entries so that unused sprites are not rendered. Resets 
 * oam_index, allowing a new set of sprites to be rendered. This function should
 * be run after rendering.
*/
void clean_oam() BANKED
{
	static u8 last_oam = 0;

	u8 tmp = oam_index;
	while(oam_index < last_oam) {
		shadow_OAM[oam_index++].y = 0;
	}
	last_oam = tmp;
	oam_index = 0;
}

void fade_to_white(u8 fade_speed) BANKED
{
	if (_cpu == CGB_TYPE) {
		while(1) {
			u8 completion = 0;
			for (u8 i = 0; i < 8 * 7;) {
				BCPS_REG = i++;
				WAIT_VRAM;
				short cur_pal = BCPD_REG;
				BCPS_REG = i++;
				WAIT_VRAM;
				cur_pal |= BCPD_REG << 8;
				u8 color[3] = {
					cur_pal & 0b11111,
					(cur_pal & 0b1111100000) >> 5,
					(cur_pal & 0b111110000000000) >> 10
				};
				for (u8 j = 0; j < 3; j++) {
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
			for (u8 i = 0; i < 8 * 7;) {
				OCPS_REG = i++;
				WAIT_VRAM;
				short cur_pal = OCPD_REG;
				OCPS_REG = i++;
				WAIT_VRAM;
				cur_pal |= OCPD_REG << 8;
				u8 color[3] = {
					cur_pal & 0b11111,
					(cur_pal & 0b1111100000) >> 5,
					(cur_pal & 0b111110000000000) >> 10
				};
				for (u8 j = 0; j < 3; j++) {
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
		for (u8 i = 0; i < 4; i++) {
			wait_vbl_done();
			wait_vbl_done();
			BGP_REG <<= 2;
			OBP0_REG <<= 2;
			OBP1_REG <<= 2;
		}
	}
}