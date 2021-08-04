#pragma once

#include <gb/gb.h>
#include <stdbool.h>
#include <stdint.h>

#include "include/hardware.h"
#include "include/vec.h"

#define WAIT_VRAM while (STAT_REG & STAT_BUSY) {}
#define VCAM_OFF 12 // + 16 for the text, -4 for the HUD

enum fx_modes {
	NO_UI,
	GAME_UI
};

extern uint8_t lcdc_buffer;
extern uint8_t oam_index;
extern uvec8 win_pos;
extern short palettes[64];
extern uint8_t fx_mode;

void vmemcpy(void *dest, uint8_t len, const void *src) NONBANKED;
void banked_vmemcpy(void *dest, uint8_t len, const void *src, uint8_t bank) NONBANKED;
void vblank() NONBANKED;
void clean_oam() BANKED;
void fade_to_white(uint8_t fade_speed) BANKED;
void render_palettes() BANKED;
void swipe_left(bool render) BANKED;
void swipe_right() BANKED;

inline void vset(uint16_t dst, uint8_t value) {
	while(STAT_REG & STAT_BUSY) {}
	*((uint8_t*)dst) = value;
}