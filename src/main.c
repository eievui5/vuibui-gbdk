#include <gb/cgb.h>
#include <gb/gb.h>
#include <rand.h>
#include <string.h>

#include "entities/luvui.h"

#include "include/entity.h"
#include "include/hardware.h"
#include "include/int.h"
#include "include/map.h"
#include "include/rendering.h"

const unsigned char tiles[] = {
	0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00,
	0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00,
	0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF,
	0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF,
};

u8 cur_keys = 0;
u8 new_keys;
u8 rel_keys;
u8 last_keys;

void main()
{
	add_VBL(&vblank);
	if (_cpu == CGB_TYPE) {
		cpu_fast();
		cgb_compatibility(); // Temporarily init first two pals to grey.
	}
	initrand(0);
	LCDC_REG = LCDC_ENABLE | LCDC_BG_ENABLE | LCDC_OBJ_ENABLE | LCDC_OBJ_16;
	memset(&entities, 0, sizeof(entity) * NB_ENTITIES);
	memset(map, 0, sizeof(map));
	generate_map();
	set_bkg_data(0, 2, tiles);
	for (u8 i = 0; i < NB_ENTITIES; i++) {
		entities.array[i].data = &luvui_data;
		entities.array[i].bank = bank_luvui;
		entities.array[i].x_pos = 1 + i;
		entities.array[i].y_pos = 1 + i;
		SWITCH_ROM_MBC1(entities.array[i].bank);
		set_sprite_data(
			i * NB_ENTITY_TILES, NB_ENTITY_TILES,
			entities.array[i].data->graphics
		);
	}
	move_entities();
	while(1) {
		last_keys = cur_keys;
		cur_keys = joypad();
		new_keys = ~last_keys & cur_keys;
		rel_keys = last_keys & ~cur_keys;

		if (cur_keys & (J_DOWN | J_UP | J_LEFT | J_RIGHT)) {
			bool moved = false;
			if (cur_keys & J_DOWN) {
				moved = try_step(0, DIR_DOWN);
			}
			else if (cur_keys & J_UP) {
				moved = try_step(0, DIR_UP);
			}
			else if (cur_keys & J_RIGHT) {
				moved = try_step(0, DIR_RIGHT);
			}
			else if (cur_keys & J_LEFT) {
				moved = try_step(0, DIR_LEFT);
			}
			
			if (moved) {
				for (u8 i = 1; i < NB_ENTITIES; i++) {
					if (entities.array[i].data)
						try_step(i, rand() & 0b11);
				}
				move_entities();
			}
		}
		
		render_entities();
		wait_vbl_done();
	}
}