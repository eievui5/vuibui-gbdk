#pragma once

#include <gb/gb.h>
#include "include/int.h"

#define WAIT_VRAM \
__asm \
	waitVRAM$: \
		ld a, (_STAT_REG) \
		and a, #2 \
		jr nz, waitVRAM$ \
__endasm
#define VCAM_OFF 16

extern u8 lcdc_buffer;
extern u8 oam_index;

void vblank();
void stat();
void clean_oam();