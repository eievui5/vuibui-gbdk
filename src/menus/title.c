#pragma bank 255

#include <gb/cgb.h>
#include <gb/incbin.h>
#include <stdint.h>

#include "include/bank.h"
#include "include/game.h"
#include "include/rendering.h"
#include "libs/vwf.h"

#include "gfx/ui/vwf_font.h"

INCBIN(crater_graphics, res/gfx/maps/crater.2bpp)
INCBIN_EXTERN(crater_graphics)
INCBIN(crater_map, res/gfx/maps/crater.map)
INCBIN_EXTERN(crater_map)

const char engine_splash[] =
"Vuiiger Engine\nWritten by Eievui\n\n"
"This is a versatile Game Boy\n"
"engine for creating turn-based\n"
"dungeon crawlers, specifically\n"
"in the style of the \"Pokemon\n"
"Mystery Dungeon\" games.\n\n"
"Press START!";

void show_title() BANKED
{
	cgb_compatibility();
	//vwf_load_font(0, vwf_font, BANK(vwf_font));
	//vwf_draw_text(1, 1, 1, engine_splash);

	set_bkg_data(0x00, 0x7F, crater_graphics);
	set_bkg_tiles(0, 0, 20, 18, crater_map);
	while (1) {
		if (cur_keys & J_START)
			break;
		wait_vbl_done();
	}
}