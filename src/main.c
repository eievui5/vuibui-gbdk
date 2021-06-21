#include <gb/gb.h>
#include <rand.h>
#include <string.h>

#include "include/entity.h"
#include "include/hardware.h"
#include "include/rendering.h"

const unsigned char tile[] = {
	0x00, 0xFF, 0xFF, 0x00, 
	0x00, 0xFF, 0xFF, 0x00, 
	0x00, 0xFF, 0xFF, 0x00, 
	0x00, 0xFF, 0xFF, 0x00
};

const unsigned char tile2[] = {
	0xFF, 0xFF, 0x00, 0xFF, 
	0xFF, 0xFF, 0x00, 0xFF, 
	0xFF, 0xFF, 0x00, 0xFF, 
	0xFF, 0xFF, 0x00, 0xFF
};

const char debug_metasprite[] = {
	0, 0,
	0, 0
};

const entity_data debug_entity_data = {
	.metasprites = debug_metasprite
};

u8 cur_keys = 0;
u8 new_keys;
u8 rel_keys;
u8 last_keys;

void main()
{
	initrand(0);
	LCDC_REG = LCDC_ENABLE | LCDC_BG_ENABLE | LCDC_OBJ_ENABLE | LCDC_OBJ_16;
	memset(&entities, 0, sizeof(entity) * NB_ENTITIES);
	for (u8 i = 0; i < NB_ENTITIES; i++) {
		entities.array[i].data = &debug_entity_data;
		entities.array[i].x_pos = 1 + i;
		entities.array[i].y_pos = 1 + i;
		if (i & 1) {
			set_sprite_data(i * 2, 1, tile);
			set_sprite_data(i * 2 + 1, 1, tile);
		} else {
			set_sprite_data(i * 2, 1, tile2);
			set_sprite_data(i * 2 + 1, 1, tile2);
		}
	}
	move_entities();
	while(1) {
		last_keys = cur_keys;
		cur_keys = joypad();
		new_keys = ~last_keys & cur_keys;
		rel_keys = last_keys & ~cur_keys;

		if (cur_keys & (J_DOWN | J_UP | J_LEFT | J_RIGHT)) {
			u16 target_x = entities.player.x_pos;
			u16 target_y = entities.player.y_pos;
			if (cur_keys & J_DOWN) {
				target_y += 1;
			}
			else if (cur_keys & J_UP) {
				target_y -= 1;
			}
			else if (cur_keys & J_RIGHT) {
				target_x += 1;
			}
			else if (cur_keys & J_LEFT) {
				target_x -= 1;
			}
			if (!check_collision(0, target_x, target_y)) {
				entities.player.x_pos = target_x;
				entities.player.y_pos = target_y;
				switch (rand() & 0b11) {
				case 0:
					entities.allies[1].y_pos += 1;
					break;
				case 1:
					entities.allies[1].y_pos -= 1;
					break;
				case 2:
					entities.allies[1].x_pos += 1;
					break;
				case 3:
					entities.allies[1].x_pos -= 1;
					break;
				}
				move_entities();
			}
		}

		render_entities();
		clean_oam();
		wait_vbl_done();
	}
}