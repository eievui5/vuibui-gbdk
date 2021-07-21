#pragma once

#include <gb/gb.h>

#include "include/hardware.h"
#include "include/int.h"
#include "include/vec.h"

#define WAIT_VRAM while (STAT_REG & STAT_BUSY) {}
#define VCAM_OFF 12 // + 16 for the text, -4 for the HUD

enum fx_modes {
	NO_UI,
	GAME_UI
};

extern u8 lcdc_buffer;
extern u8 oam_index;
extern uvec8 win_pos;
extern short palettes[64];
extern u8 fx_mode;

void vmemcpy(void *dest, u8 len, const void *src) NONBANKED;
void banked_vmemcpy(void *dest, u8 len, const void *src, u8 bank) NONBANKED;
void vblank() NONBANKED;
void clean_oam() BANKED;
void fade_to_white(u8 fade_speed) BANKED;
void render_palettes() BANKED;
void swipe_left() BANKED;
void swipe_right() BANKED;

inline void vset(uint16_t dst, uint8_t value) {
	while(STAT_REG & STAT_BUSY) {}
	*((uint8_t*)dst) = value;
}