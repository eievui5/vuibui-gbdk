#pragma once

#include <gb/incbin.h>

#define MESSAGE_SIZE 60u

INCBIN_EXTERN(hud_tiles)
INCBIN_EXTERN(font_tiles)
INCBIN_EXTERN(health_tiles)

typedef struct {
	short colors[4];
	unsigned char gradient_start[3];
} ui_pal;

void clear_print_hud() BANKED;
void init_hud() BANKED;
void draw_clock() BANKED;
void draw_move_window() NONBANKED;
void draw_dir_window() BANKED;
void show_hud() NONBANKED;
void print_hud(const char *src) BANKED;