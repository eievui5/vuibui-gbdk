#pragma bank 255

#include <gb/cgb.h>
#include <gb/gb.h>
#include <gb/incbin.h>

#include "gfx/ui/vwf_font.h"
#include "include/hardware.h"
#include "include/hud.h"
#include "include/map.h"
#include "include/rendering.h"
#include "libs/vwf.h"

INCBIN(font_tiles, res/gfx/ui/font.1bpp.2bpp)
INCBIN(hud_tiles, res/gfx/ui/hud.2bpp)

#define FONT_TILE (0x80 + SIZE(hud_tiles) / 16 + 1)
#define FONT_SPACE ((void *)(0x8800 + FONT_TILE * 16))

const unsigned char hud[] = {
	0x80, 0x82, 0x83, 0x84, 0x84, 0x84, 0x84, 0x84, 0x84, 0x84,
	0x84, 0x84, 0x84, 0x84, 0x85, 0x87, 0x88, 0x89, 0x81, 0x86
};
const short hud_palettes[] = {
	RGB_WHITE, RGB_BLUE, RGB_DARKBLUE, RGB_BLACK
};

void init_hud() BANKED
{
	u8 i;
	set_bkg_data(0x80, SIZE(hud_tiles) / 16, hud_tiles);
	vmemset(FONT_SPACE, 0, MESSAGE_SIZE * 16);

	vmemcpy((void *)(0x9F60), 20, hud);

	vmemset((void *)(0x9F80), 0x8A, 20);
	for (i = 0; i < 3; i++) {
		vmemset((void *)(0x9FA0 + i * 32), 0x8B, MESSAGE_SIZE / 3);
	}

	if (_cpu == CGB_TYPE) {
		set_bkg_palette(7, 1, hud_palettes);
		VBK_REG = 1;
		vmemset((void *)(0x9F60), 7, 160);
		VBK_REG = 0;
	}

	vwf_load_font(0, vwf_font, BANK(vwf_font));
}

// Need to declare this...
void show_game() NONBANKED;

void show_hud() NONBANKED
{
	WAIT_VRAM;
	LCDC_REG = LCDC_ENABLE | LCDC_BG_ENABLE | LCDC_OBJ_16 | LCDC_BG_SCRN1;
	SCX_REG = 0;
	SCY_REG = 216;

	// remove_LCD(&show_hud); // show_hud uses VBlank, not STAT.
	LYC_REG = 7;
	add_LCD(&show_game);
}

void show_text() NONBANKED
{
	WAIT_VRAM;
	// Disable objects and swap to map 2.
	LCDC_REG = LCDC_ENABLE | LCDC_BG_ENABLE | LCDC_OBJ_16 | LCDC_BG_SCRN1;
	SCX_REG = 0;
	SCY_REG = 112;

	remove_LCD(&show_text);
}

void show_game() NONBANKED
{
	WAIT_VRAM;
	LCDC_REG = lcdc_buffer;
	SCX_REG = camera.x;
	SCY_REG = camera.y;
	
	remove_LCD(&show_game);
	LYC_REG = SCREENHEIGHT - 33;
	add_LCD(&show_text);
}


void print_hud(const char *src) BANKED
{
	for (u8 i = 0; i < 3; i++)
		vmemset((void *)(0x9FA1 + i * 32), FONT_TILE - 1, MESSAGE_SIZE / 3);
	vwf_activate_font(0);
	vwf_draw_text(0x01, 0x1D, FONT_TILE, src);
	//u8 c = 0;
	//for (u8 i = 0; src[i] != 0; i++, c++) {
	//	if (src[i] == '\n') {
	//		// Clear the remaining tiles on this line.
	//		vmemset(
	//			(void *)(0x8850 + c * 16), 
	//			0, 
	//			((c + (MESSAGE_SIZE / 3 - c % (MESSAGE_SIZE / 3))) - c) * 16
	//		);
	//		// Jump to the next line.
	//		c += MESSAGE_SIZE / 3 - c % (MESSAGE_SIZE / 3);
	//		i++;
	//	}
	//	set_bkg_1bit_data(0x85 + c, 1, &font_tiles[(src[i] - ' ') * 8], 3);
	//}
	//vmemset((void *)(0x8850 + c * 16), 0, (MESSAGE_SIZE - c) * 16);
}