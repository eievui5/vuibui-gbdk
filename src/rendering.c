#include <gb/gb.h>
#include "include/hud.h"
#include "include/int.h"

u8 lcdc_buffer;
u8 oam_index = 0;

void vblank()
{
	show_hud();
}

void vmemcpy(void *dest, u8 len, const void *src)
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
void clean_oam()
{
	static u8 last_oam = 0;

	u8 tmp = oam_index;
	while(oam_index < last_oam) {
		shadow_OAM[oam_index++].y = 0;
	}
	last_oam = tmp;
	oam_index = 0;
}