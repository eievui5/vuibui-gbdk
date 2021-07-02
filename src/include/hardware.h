#pragma once

#include <gb/hardware.h>

#define LCDC_ENABLE 		0b10000000u
#define LCDC_WINDOW_SCRN1 	0b01000000u
#define LCDC_WINDOW_ENABLE 	0b00100000u
#define LCDC_BGTILES_8000 	0b00010000u
#define LCDC_BG_SCRN1 		0b00001000u
#define LCDC_OBJ_16 		0b00000100u
#define LCDC_OBJ_ENABLE 	0b00000010u
#define LCDC_BG_ENABLE 		0b00000001u

#define OAM_PRIORITY		0b10000000u
#define OAM_YFLIP		0b01000000u
#define OAM_XFLIP		0b00100000u
#define OAM_DMG_PAL0		0b00000000u
#define OAM_DMG_PAL1		0b00010000u
#define OAM_VRAM_BANK0		0b00000000u
#define OAM_VRAM_BANK1		0b00001000u
#define OAM_CGB_PAL_MASK	0b00000111u

#define STATF_LYC    		0b01000000 // LYC=LY Coincidence (Selectable)
#define STATF_MODE10 		0b00100000 // Mode 10
#define STATF_MODE01 		0b00010000 // Mode 01 (V-Blank)
#define STATF_MODE00 		0b00001000 // Mode 00 (H-Blank)
#define STATF_LYCF   		0b00000100 // Coincidence Flag
#define STATF_HBL    		0b00000000 // H-Blank
#define STATF_VBL    		0b00000001 // V-Blank
#define STATF_OAM    		0b00000010 // OAM-RAM is used by system
#define STATF_LCD    		0b00000011 // Both OAM and VRAM used by system
#define STATF_BUSY   		0b00000010 // When set, VRAM access is unsafe