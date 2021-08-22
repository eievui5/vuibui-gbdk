#pragma once

#include <gb/gb.h>
#include <stdint.h>

#define __VWF_BANK_PREFIX(A) __bank_##A
#define TO_VWF_FARPTR(A) {.bank = (char)&(__VWF_BANK_PREFIX(A)), .ptr = (char*) &(A)}

#define RECODE_7BIT 1
#define FONT_VWF 2
#define FONT_VWF_1BIT 4

typedef struct vwf_farptr_t {
	uint8_t bank;
	void * ptr;
} vwf_farptr_t;

typedef struct font_desc_t {
	uint8_t attr;
	const uint8_t * recode_table;
	const uint8_t * widths;
	const uint8_t * bitmaps;
} font_desc_t;

extern vwf_farptr_t vwf_fonts[4];

void vwf_load_font(uint8_t idx, const void * font, uint8_t bank);
void vwf_activate_font(uint8_t idx);
void vwf_draw_text(uint8_t x, uint8_t y, uint8_t* base_pointer,
		   uint8_t base_tile, const unsigned char * str);
void vwf_wrap_str(uint8_t pixel_width, unsigned char * str);
uint8_t vwf_next_tile();