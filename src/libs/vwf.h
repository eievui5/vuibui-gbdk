#ifndef _VWF_H_INCLUDE
#define _VWF_H_INCLUDE

#include <gb/gb.h>

#include "include/int.h"

#define __VWF_BANK_PREFIX(A) __bank_##A
#define TO_VWF_FARPTR(A) {.bank = (char)&(__VWF_BANK_PREFIX(A)), .ptr = (void *)&(A)}

#define RECODE_7BIT 1
#define FONT_VWF 2
#define FONT_VWF_1BIT 4

typedef struct vwf_farptr_t {
    UINT8 bank;
    void * ptr;
} vwf_farptr_t;

typedef struct font_desc_t {
    u8 attr;
    const u8 * recode_table;
    const u8 * widths;
    const u8 * bitmaps;
} font_desc_t;

extern vwf_farptr_t vwf_fonts[4];

void vwf_load_font(u8 idx, const void * font, u8 bank);
void vwf_activate_font(u8 idx);
void vwf_draw_text(u8 x, u8 y, u8 base_tile, const unsigned char * str);
void vwf_wrap_str(u8 pixel_width, unsigned char * str);
u8 vwf_next_tile();

#endif