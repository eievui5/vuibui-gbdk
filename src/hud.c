#pragma bank 255

#include <gb/gb.h>
#include <gb/incbin.h>
#include "include/hardware.h"
#include "include/hud.h"
#include "include/map.h"
#include "include/rendering.h"

INCBIN(font_tiles, res/gfx/ui/font.1bpp.2bpp)
INCBIN(health_tiles, res/gfx/ui/health_bar.2bpp)
INCBIN(hud_tiles, res/gfx/ui/hud_tiles.2bpp)

const unsigned char hud[] = {
	0xBB, 0xBD, 0xBE, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF, 0xC0, 0xC2, 0xC3, 0xC4, 0xBC, 0xC1
};

void init_hud() BANKED
{
	u8 i;
	set_bkg_data(0x80, SIZE(hud_tiles) / 16, hud_tiles);
	vmemset((void *)(0x8850), 0, MESSAGE_SIZE * 16);
	set_bkg_data(0xBB, SIZE(health_tiles) / 16, health_tiles);
	
	for (i = 0; i < 20; i++)
		set_vram_byte((void *)(0x9F60 + i), hud[i]);

	set_vram_byte((void *)(0x9C00 + 28 * 32), 0x80);
	set_vram_byte((void *)(0x9C00 + 28 * 32 + 19), 0x82);
	for (i = 1; i < 19; i++)
		set_vram_byte((void *)(0x9C00 + 28 * 32 + i), 0x81);
	u8 font_tile = 0x85;
	for (i = 0; i < 3; i++) {
		set_vram_byte((void *)(0x9C00 + (29 + i) * 32), 0x83);
		set_vram_byte((void *)(0x9C00 + (29 + i) * 32 + 19), 0x84);
		for (u8 j = 1; j < 19; j++)
			set_vram_byte(
				(void *)(0x9C00 + (29 + i) * 32 + j),
				font_tile++
			);
	}
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
	SCX_REG = camera_x;
	SCY_REG = camera_y;
	
	remove_LCD(&show_game);
	LYC_REG = SCREENHEIGHT - 33;
	add_LCD(&show_text);
}


void print_hud(const char *src) BANKED
{
	u8 c = 0;
	for (u8 i = 0; src[i] != 0; i++, c++) {
		if (src[i] == '\n') {
			// Clear the remaining tiles on this line.
			vmemset(
				(void *)(0x8850 + c * 16), 
				0, 
				((c + (MESSAGE_SIZE / 3 - c % (MESSAGE_SIZE / 3))) - c) * 16
			);
			// Jump to the next line.
			c += MESSAGE_SIZE / 3 - c % (MESSAGE_SIZE / 3);
			i++;
		}
		set_bkg_1bit_data(0x85 + c, 1, &font_tiles[(src[i] - ' ') * 8], 3);
	}
	vmemset((void *)(0x8850 + c * 16), 0, (MESSAGE_SIZE - c) * 16);
}