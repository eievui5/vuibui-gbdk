#pragma bank 255

#include <gb/cgb.h>
#include <gb/gb.h>
#include <gb/incbin.h>

#include "gfx/ui/vwf_font.h"
#include "include/entity.h"
#include "include/hardware.h"
#include "include/hud.h"
#include "include/map.h"
#include "include/move.h"
#include "include/rendering.h"
#include "libs/vwf.h"

INCBIN(font_tiles, res/gfx/ui/font.1bpp.2bpp)
INCBIN(hud_tiles, res/gfx/ui/hud.2bpp)
INCBIN(arrow_tiles, res/gfx/ui/arrows.2bpp)
INCBIN_EXTERN(arrow_tiles)

#define HUD_TILE 0x80u
#define ARROW_TILE (0x80 + SIZE(hud_tiles) / 16)
#define FONT_TILE (0x80 + (SIZE(hud_tiles) + SIZE(arrow_tiles))/ 16 + 1)
#define MOVE_TILE (FONT_TILE + MESSAGE_SIZE)

#define TILEADDR(t) ((0x8800 + ((t) - 0x80u) * 16))

const unsigned char hud[] = {
	HUD_TILE + 0u, HUD_TILE + 2u, HUD_TILE + 3u, HUD_TILE + 4u, 
	HUD_TILE + 4u, HUD_TILE + 4u, HUD_TILE + 4u, HUD_TILE + 4u, 
	HUD_TILE + 4u, HUD_TILE + 4u, HUD_TILE + 4u, HUD_TILE + 4u, 
	HUD_TILE + 4u, HUD_TILE + 4u, HUD_TILE + 5u, HUD_TILE + 7u, 
	HUD_TILE + 8u, HUD_TILE + 9u, HUD_TILE + 1u, HUD_TILE + 6u,
};
const short hud_palettes[] = {
	RGB_WHITE, RGB_BLUE, RGB_DARKBLUE, RGB_BLACK
};

void init_hud() BANKED
{
	u8 i;
	vmemcpy((void *)TILEADDR(HUD_TILE), SIZE(hud_tiles), hud_tiles);
	vmemcpy((void *)TILEADDR(ARROW_TILE), SIZE(arrow_tiles), arrow_tiles);

	// Load hud
	vmemcpy((void *)(0x9F60), 20, hud);

	// Setup text box
	vmemset((void *)(0x9F80), 0x8A, 20);
	for (i = 0; i < 3; i++) {
		vmemset((void *)(0x9FA0 + i * 32), FONT_TILE - 1, MESSAGE_SIZE / 3);
	}

	// Setup attack window
	vset(0x9C00, HUD_TILE + 12u);
	vmemset((void *)(0x9C01), HUD_TILE + 10u, 31);
	for (i = 1; i < 5; i++) {
		vset(0x9C00 + i * 32, HUD_TILE + 11u);
		vset(0x9C01 + i * 32, ARROW_TILE-1 + i);
		vmemset((void *)(0x9C02 + i * 32), FONT_TILE - 1, 30);
	}

	if (_cpu == CGB_TYPE) {
		set_bkg_palette(7, 1, hud_palettes);
		VBK_REG = 1;
		vmemset((void *)(0x9C00), 7, 1024);
		VBK_REG = 0;
	}

	vwf_load_font(0, vwf_font, BANK(vwf_font));
	vwf_activate_font(0);
}

/**
 * Draws the names of the player's moves to the move window. Update whenever
 * moves change.
*/
void init_move_window() BANKED
{
	move *moves = entities.player.moves;
	if (moves->data)
		vwf_draw_text(2, 1, MOVE_TILE, moves->data->name);
	moves++;
	for (u8 i = 1; i < 4; i++) {
		if (moves->data)
			vwf_draw_text(2, i + 1, vwf_next_tile(), moves->data->name);
		moves++;
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