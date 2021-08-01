#pragma bank 255

#include <gb/cgb.h>
#include <gb/gb.h>
#include <gb/incbin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "include/entity.h"
#include "include/game.h"
#include "include/hardware.h"
#include "include/hud.h"
#include "include/map.h"
#include "include/move.h"
#include "include/rendering.h"
#include "libs/vwf.h"

#include "gfx/ui/vwf_font.h"
#include "gfx/ui/clockface_font.h"

INCBIN(hud_tiles, res/gfx/ui/hud.2bpp)
INCBIN(arrow_tiles, res/gfx/ui/arrows.2bpp)
INCBIN_EXTERN(arrow_tiles)

#define HUD_TILE (0x80u)
#define ARROW_TILE (0x8Au)
#define FONT_TILE (0x8Fu)
#define CLOCK_TILE (FONT_TILE + MESSAGE_SIZE)
#define MOVE_TILE (CLOCK_TILE + 3u)

#define TILEADDR(t) ((0x8800u + ((t) - 0x80u) * 16u))

const unsigned char hud[] = {
	HUD_TILE + 0u, HUD_TILE + 2u, HUD_TILE + 3u, HUD_TILE + 4u, 
	HUD_TILE + 4u, HUD_TILE + 4u, HUD_TILE + 4u, HUD_TILE + 4u, 
	HUD_TILE + 4u, HUD_TILE + 4u, HUD_TILE + 4u, HUD_TILE + 4u, 
	HUD_TILE + 4u, HUD_TILE + 4u, HUD_TILE + 5u, HUD_TILE + 1u, 
	HUD_TILE + 1u, HUD_TILE + 1u, HUD_TILE + 1u, HUD_TILE + 6u,
};
const unsigned char arrow_window[] = {
	FONT_TILE - 1u, ARROW_TILE, FONT_TILE - 1u, FONT_TILE - 1u, 
	FONT_TILE - 1u, ARROW_TILE + 3u, FONT_TILE - 1u, ARROW_TILE + 1u, 
	FONT_TILE - 1u, FONT_TILE - 1u, FONT_TILE - 1u, ARROW_TILE + 2u, 
	FONT_TILE - 1u, FONT_TILE - 1u, FONT_TILE - 1u,
};

// Default UI colors.
const ui_pal default_ui_white = {
	.colors = {RGB_WHITE, RGB_LIGHTGRAY, RGB_DARKGRAY, RGB_BLACK},
	.gradient_start = {31, 31, 31}
};
const ui_pal default_ui_black = {
	.colors = {RGB(4, 4, 4), RGB(8, 8, 8), RGB(16, 16, 16), RGB_WHITE},
	.gradient_start = {8, 8, 8}
};
const ui_pal default_ui_red = {
	.colors = {RGB(31, 20, 20), RGB_RED, RGB_DARKRED, RGB_BLACK},
	.gradient_start = {31, 8, 12}
};
const ui_pal default_ui_green = {
	.colors = {RGB(20, 31, 20), RGB_GREEN, RGB_DARKGREEN, RGB_BLACK},
	.gradient_start = {12, 31, 8}
};
const ui_pal default_ui_blue = {
	.colors = {RGB(20, 20, 31), RGB_BLUE, RGB_DARKBLUE, RGB_BLACK},
	.gradient_start = {8, 12, 31}
};
const ui_pal default_ui_pink = {
	.colors = {RGB(31, 20, 31), RGB(31, 3, 31), RGB(16, 0, 16), RGB_BLACK},
	.gradient_start = {31, 3, 31}
};
const ui_pal default_ui_festive = {
	.colors = {RGB(20, 31, 20), RGB_RED, RGB_DARKRED, RGB_BLACK},
	.gradient_start = {12, 31, 8}
};
const ui_pal default_ui_grape = {
	.colors = {RGB(31, 31, 16), RGB_PURPLE, RGB(12, 0, 12), RGB_BLACK},
	.gradient_start = {24, 6, 24}
};
const ui_pal default_ui_retro = {
	.colors = {RGB(31, 24, 12), RGB(24, 12, 6), RGB(12, 6, 0), RGB_BLACK},
	.gradient_start = {8, 12, 31}
};
const ui_pal default_ui_dmg = {
	.colors = {RGB(26, 26, 9), RGB(17, 21, 9), RGB(8, 14, 8), RGB(0, 6, 4)},
	.gradient_start = {20, 26, 10}
};

ui_pal current_ui_pal;
uint8_t status_position = 8;
uint8_t text_position = 32;

void init_hud() BANKED
{
	memcpy(&current_ui_pal, &default_ui_blue, sizeof(ui_pal));
	uint8_t i;
	vmemcpy((void *)TILEADDR(HUD_TILE), SIZE(hud_tiles), hud_tiles);
	vmemcpy((void *)TILEADDR(ARROW_TILE), SIZE(arrow_tiles), arrow_tiles);

	// Load hud
	vmemcpy((void *)(0x9F60), 20, hud);

	// Setup text box
	vmemset((void *)(0x9F80), HUD_TILE + 7u, 20);
	for (i = 0; i < 3; i++) {
		vmemset((void *)(0x9FA0 + i * 32), FONT_TILE - 1u, MESSAGE_SIZE / 3);
	}

	// Setup attack window
	vset(0x9C00, HUD_TILE + 9u);
	vmemset((void *)(0x9C01), HUD_TILE + 7u, 31);
	for (i = 1; i < 5; i++) {
		vset(0x9C00 + i * 32, HUD_TILE + 8u);
	}

	if (_cpu == CGB_TYPE) {
		set_bkg_palette(7, 1, current_ui_pal.colors);
		VBK_REG = 1;
		vmemset((void *)(0x9C00), 7, 1024);
		VBK_REG = 0;
	}

	vwf_load_font(0, vwf_font, BANK(vwf_font));
	vwf_load_font(1, clockface_font, BANK(clockface_font));
	draw_clock();
}

/**
 * Draws the names of the player's moves to the move window.
*/
void draw_move_window() NONBANKED
{
	uint8_t i = 1;
	for (; i < 5; i++) {
		vset(0x9C01 + i * 32, ARROW_TILE - 1u + i);
		vmemset((void *)(0x9C02 + i * 32), FONT_TILE - 1u, 8);
	}

	uint8_t temp_bank = _current_bank;

	move *moves = PLAYER.moves;
	if (moves->data) {
		SWITCH_ROM_MBC1(moves->bank);
		vwf_draw_text(2, 1, MOVE_TILE, moves->data->name);
	}
	moves++;
	for (i = 1; i < 4; i++) {
		if (moves->data) {
			SWITCH_ROM_MBC1(moves->bank);
			vwf_draw_text(2, i + 1, vwf_next_tile(), 
				      moves->data->name);
		}
		moves++;
	}

	SWITCH_ROM_MBC1(temp_bank);
}

/**
 * Draws the direction menu.
*/
void draw_dir_window() BANKED
{
	vmemcpy((void *)(0x9C21), 5, &arrow_window[0]);
	vmemcpy((void *)(0x9C41), 5, &arrow_window[5]);
	vmemcpy((void *)(0x9C61), 5, &arrow_window[10]);
	vmemset((void *)(0x9C81), FONT_TILE - 1u, 5);
}

// Need to declare this...
void show_game() NONBANKED;

void show_hud() NONBANKED
{
	WAIT_VRAM;
	LCDC_REG = LCDC_ENABLE | LCDC_BG_ENABLE | LCDC_OBJ_16 | LCDC_BG_SCRN1;
	SCX_REG = 0;
	SCY_REG = 216 + (8 - status_position);

	if (_cpu == CGB_TYPE) {
		BCPS_REG = 7 * 8 | 0x80;
		BCPD_REG = current_ui_pal.colors[0] & 0xFF;
		BCPD_REG = (current_ui_pal.colors[0] & 0xFF00) >> 8;
	}

	// remove_LCD(&show_hud); // show_hud uses VBlank, not STAT.
	LYC_REG = status_position - 1;
	add_LCD(&show_game);
}

void hi_color() NONBANKED
{
	uint8_t r = current_ui_pal.gradient_start[0] - ((LYC_REG - 112) >> 1);
	uint8_t g = current_ui_pal.gradient_start[1] - ((LYC_REG - 112) >> 1);
	uint8_t b = current_ui_pal.gradient_start[2] - ((LYC_REG - 112) >> 1);
	if (r > 31)
		r = 0;
	if (g > 31)
		g = 0;
	if (b > 31)
		b = 0;
	uint8_t clr0 = r | g << 5;
	uint8_t clr1 = g >> 3 | b << 2;
	BCPS_REG = 7 * 8 | 0x80;
	LYC_REG += 2;
	bool done = false;
	WAIT_VRAM;
	BCPD_REG = clr0;
	BCPD_REG = clr1;
	if (LYC_REG >= 143) {
		remove_LCD(&hi_color);
	}
}

void show_text() NONBANKED
{
	WAIT_VRAM;
	// Disable objects and swap to map 2.
	LCDC_REG = LCDC_ENABLE | LCDC_BG_ENABLE | LCDC_OBJ_16 | LCDC_BG_SCRN1;
	SCX_REG = 0;
	SCY_REG = 112 - (32 - text_position);

	remove_LCD(&show_text);
	if (_cpu == CGB_TYPE) {
		add_LCD(&hi_color);
		LYC_REG += 2;
	}
}

void show_game() NONBANKED
{
	WAIT_VRAM;
	LCDC_REG = lcdc_buffer;
	SCX_REG = camera.x;
	SCY_REG = camera.y;

	remove_LCD(&show_game);
	LYC_REG = SCREENHEIGHT - 1 - text_position;
	add_LCD(&show_text);
}

void clear_print_hud() BANKED
{
	for (uint8_t i = 0; i < 3; i++)
		vmemset((void *)(0x9FA1 + i * 32), FONT_TILE - 1u,
			MESSAGE_SIZE / 3);
}

void print_hud(const char *src) BANKED
{
	clear_print_hud();
	vwf_activate_font(0);
	vwf_draw_text(0x01, 0x1D, FONT_TILE, src);
}

const char clock_string[] = "%u:%s";
const char minute_string[] = "%u";
void draw_clock() BANKED
{
	vwf_activate_font(1);
	char minute_buffer[3] = {'0'};
	if (minutes < 10)
		sprintf(&minute_buffer[1], minute_string, minutes);
	else
		sprintf(minute_buffer, minute_string, minutes);
	char clock_buffer[6];
	sprintf(clock_buffer, clock_string, hours, minute_buffer);
	vwf_draw_text(15, 27, CLOCK_TILE, clock_buffer);
	vwf_activate_font(0);
	vmemset((void *)(0x800Du + CLOCK_TILE * 16u), 0xFF, 3);
	vmemset((void *)(0x801Du + CLOCK_TILE * 16u), 0xFF, 3);
	vmemset((void *)(0x802Du + CLOCK_TILE * 16u), 0xFF, 3);
}
