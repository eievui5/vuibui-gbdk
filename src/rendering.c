#include <gb/gb.h>
#include "include/int.h"

u8 oam_index = 0;
u8 last_oam = 0;

/**
 * Cleans leftover oam entries so that unused sprites are not rendered. Resets 
 * oam_index, allowing a new set of sprites to be rendered. This function should
 * be run after rendering.
*/
void clean_oam() {
	u8 tmp = oam_index;
	while(oam_index < last_oam) {
		shadow_OAM[oam_index++].y = 0;
	}
	last_oam = tmp;
	oam_index = 0;
}