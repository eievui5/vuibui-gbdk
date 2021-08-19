#pragma bank 255

#include <gb/cgb.h>
#include <gb/incbin.h>

#include "include/hardware.h"
#include "include/game.h"
#include "include/rendering.h"
#include "libs/vwf.h"

#include "gfx/ui/vwf_font.h"

const char engine_splash[] =
"Vuiiger Engine\n"
"Written by Eievui\n"
"\n"
"This is a versatile Game Boy\n"
"engine for creating turn-based\n"
"dungeon crawlers, specifically\n"
"in the style of the \"Pokemon\n"
"Mystery Dungeon\" games.\n\n"
"Press START!";

void init_title() BANKED
{
	lcdc_buffer = \
		LCDC_ENABLE | LCDC_BG_ENABLE | LCDC_BG_SCRN1 | \
		LCDC_OBJ_ENABLE | LCDC_OBJ_16;
	cgb_compatibility();
	vwf_load_font(0, vwf_font, BANK(vwf_font));
	vwf_draw_text(1, 1, 1, engine_splash);
}

void simulate_title() BANKED
{
	if (cur_keys & J_START)
		game_state = WORLDMAP_STATE;
}