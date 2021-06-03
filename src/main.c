#include <gb/gb.h>
#include <gb/hardware.h>
#include <stdint.h>
#include <string.h>

#include "include/map.h"
#include "include/entity.h"
#include "include/hardware_flags.h"
#include "include/rendering.h"
#include "include/vector.h"

#define camera_speed 1

const unsigned char tiles[] = {
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	
	0xF0, 0x0F, 0xF0, 0x0F, 0xF0, 0x0F, 0xF0, 0x0F, 
	0xF0, 0x0F, 0xF0, 0x0F, 0xF0, 0x0F, 0xF0, 0x0F,
};

const unsigned char map[] = {
	0x00, 0x01, 0x01,
	0x01, 0x02, 0x01,
	0x01, 0x01, 0x00,
};

const entity_frame metasprite[] = {
	{0, 0, 0, 0}
};

const entity_data test = {
	.bank = 0,
	.metasprite = metasprite
};

uint8_t cur_keys = 0, last_keys, new_keys, released_keys;

void main(void) {
	// Set up some initial tiles
	set_bkg_data(1, 2, tiles);
	set_sprite_data(0, 1, tiles);
	set_bkg_tiles(0, 0, 3, 3, map);

	memset(entity_array, 0, MAX_ENTITIES * sizeof(entity));
	{
		entity new = { .data = &test, .x = 0, .y = 0};
		entity_array[0] = new;
	}

	// Enable the Screen and Background
	LCDC_REG = LCDC_ENABLE | LCDC_BG_ENABLE | LCDC_OBJ_ENABLE | LCDC_OBJ_16;
	//ENABLE_OAM_DMA;

	while (1) {
		// Input
		last_keys = cur_keys;
		cur_keys = joypad();
		new_keys = cur_keys & ~last_keys;
		released_keys = last_keys & ~cur_keys;
		oam_index = 0;

		if (new_keys & J_DOWN) { entity_array[0].y += 1; }
		if (new_keys & J_UP) { entity_array[0].y -= 1; }
		if (new_keys & J_RIGHT) { entity_array[0].x += 1; }
		if (new_keys & J_LEFT) { entity_array[0].x -= 1; }

		render_entity_array();

		wait_vbl_done();

		SCX_REG = camera_position.x; 
		SCY_REG = camera_position.y; 

		// scroll_map();
	}
}