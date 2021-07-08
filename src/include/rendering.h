#pragma once

#include <gb/gb.h>

#include "include/hardware.h"
#include "include/int.h"
#include "include/vec.h"

#define WAIT_VRAM while (STAT_REG & STAT_BUSY) {}

inline void vset(uint16_t dst, uint8_t value) {
    while(STAT_REG & STAT_BUSY) {}
    *((uint8_t*)dst) = value;
}

#define VCAM_OFF 12 // + 16 for the text, -4 for the HUD

extern u8 lcdc_buffer;
extern u8 oam_index;
extern uvec8 win_pos;

void vmemcpy(void *dest, u8 len, const void *src);
void vblank();
void stat();
void clean_oam();