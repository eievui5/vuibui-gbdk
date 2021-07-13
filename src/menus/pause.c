#pragma bank 255

#include "include/bank.h"
#include "include/entity.h"
#include "include/hud.h"
#include "include/game.h"
#include "include/int.h"
#include "include/rendering.h"

DEF_BANK(pause_menu)

u8 pause_menu()
{
	// Init.
	clear_print_hud();

	fade_to_white(1);

	//while(1) {
	//	update_input();
	//	if (new_keys == J_START) {
	//		goto exit;
	//	}
	//	wait_vbl_done();
	//}

	exit:
	return 0;
}