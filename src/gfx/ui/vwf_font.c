#pragma bank 255

// VWF Font

#include <gb/incbin.h>
#include "libs/vwf.h"

static const u8 font_table[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F, 
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F     
};

static const u8 font_widths[] = {
    3, 3, 5, 6, 5, 5, 7, 3, 3, 3, 6, 5, 3, 5, 2, 5,
    5, 4, 5, 5, 5, 5, 5, 5, 5, 5, 3, 3, 4, 5, 4, 5,
    8, 6, 5, 5, 5, 6, 6, 6, 6, 4, 6, 6, 6, 8, 6, 6, 
    5, 6, 6, 6, 6, 6, 6, 8, 6, 6, 6, 4, 5, 4, 6, 5,
    3, 5, 5, 5, 5, 5, 4, 5, 5, 3, 4, 5, 4, 6, 5, 5,
    5, 5, 5, 5, 4, 5, 6, 6, 5, 5, 4, 5, 2, 4, 6, 6
};

INCBIN(font_bitmaps, res/gfx/ui/OptixComic_vx8.2bpp)
INCBIN_EXTERN(font_bitmaps)

const void __at(255) __bank_vwf_font;
const font_desc_t vwf_font = {
    RECODE_7BIT | FONT_VWF | FONT_VWF_1BIT, 
    font_table,
    font_widths,
    font_bitmaps
};