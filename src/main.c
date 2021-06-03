#include <gb/gb.h>
#include <gb/hardware.h>
#include <stdint.h>
#include <string.h>

#include "include/collision.h"
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
	0x02, 0x01,
	0x01, 0x02
};

const entity_frame metasprite[] = {
	{0, 0, 0, 0}
};

const entity_data test = {
	.metasprite = metasprite
};

uint8_t cur_keys = 0, last_keys, new_keys, released_keys;

void main(void) {
	// Set up some initial tiles
	set_bkg_data(1, 2, tiles);
	set_sprite_data(0, 1, tiles);
	set_bkg_tiles(0, 0, 2, 2, map);

	// Clear entity arrays
	memset(player_array, 0, MAX_ALLIES * sizeof(entity));
	memset(entity_array, 0, MAX_ENTITIES * sizeof(entity));
	{
		entity new = { &test };
		PLAYER = new;
	}
	for (uint8_t i = 0; i < 1; i++) {
		entity new = { &test };
		entity_array[i] = new;
	}

	// Enable the Screen and Background
	LCDC_REG = LCDC_ENABLE | LCDC_BG_ENABLE | LCDC_OBJ_ENABLE | LCDC_OBJ_16;

	// Main loop
	while (1) {
		// Input
		last_keys = cur_keys;
		cur_keys = joypad();
		new_keys = cur_keys & ~last_keys;
		released_keys = last_keys & ~cur_keys;

		if (cur_keys) {
			if (cur_keys & J_DOWN) {
				if (get_collision(PLAYER.x, PLAYER.y + 1) == CLEAR_COLLISION) {
					PLAYER.y += 1;
					PLAYER.direction = DIR_DOWN;
					PLAYER.action = MOVE_ACTION;
				}
			}
			else if (cur_keys & J_UP) {
				if (get_collision(PLAYER.x, PLAYER.y - 1) == CLEAR_COLLISION) {
					PLAYER.y -= 1;
					PLAYER.direction = DIR_UP;
					PLAYER.action = MOVE_ACTION;
				}
			}
			else if (cur_keys & J_RIGHT) {
				if (get_collision(PLAYER.x + 1, PLAYER.y) == CLEAR_COLLISION) {
					PLAYER.x += 1;
					PLAYER.direction = DIR_RIGHT;
					PLAYER.action = MOVE_ACTION;
				}
			}
			else if (cur_keys & J_LEFT) {
				if (get_collision(PLAYER.x - 1, PLAYER.y) == CLEAR_COLLISION) {
					PLAYER.x -= 1;
					PLAYER.direction = DIR_LEFT;
					PLAYER.action = MOVE_ACTION;
				}
			}

			process_entity_array();
		}
		camera_position.x = (PLAYER.x << 4) - SCREENWIDTH/2 + 16;
		camera_position.y = (PLAYER.y << 4) - SCREENHEIGHT/2 + 8;
		oam_index = 0;
		render_entity_array();

		wait_vbl_done();

		SCX_REG = camera_position.x; 
		SCY_REG = camera_position.y; 

		// scroll_map();
	}
}