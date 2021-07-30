#pragma bank 255

#include "include/game.h"
#include "libs/vwf.h"
#include "menus/pause.h"

const char options_list[] = "Return\n";

void options_menu() BANKED
{
	vwf_draw_text(24, 1, SUBFONT_TILE, options_list);
	slide_into_submenu_x();
	while (1) {
		update_input();
		if (new_keys == J_B)
			break;
		wait_vbl_done();
	}
	slide_out_submenu_x();
}