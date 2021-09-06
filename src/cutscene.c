/* Cutscenes play in a fixed 32 x 32 tile area with up to 6 entities which can
move around and be animated. Animations can continue through dialogue or
pause until the text is complete.
*/

/* Cutscene scripts are written using C macros and do not have local branching
capabilities. Branching must be done be creating new scripts and jumping to
them.
*/
#pragma bank 255

#include <stdbool.h>
#include <string.h>

#include "include/bank.h"
#include "include/cutscene.h"
#include "include/dialogue.h"
#include "include/dir.h"
#include "include/entity.h"
#include "include/rendering.h"

#define NB_CUTSCENE_ENTITIES 6

// Entity rendering parameters.
typedef struct {
	uint8_t bank;
	const entity_data* data;
	uint8_t y;
	uint8_t x;
	uint8_t dir;
	uint8_t frame;
	uint8_t anim_timer;
} cutscene_entity;

uint8_t cur_script_bank;
int* cur_script;
cutscene_entity cutscene_entities[NB_CUTSCENE_ENTITIES];
// Return value for script commands.
int script_return;

void init_cutscene() BANKED
{
	memset(cutscene_entities, 0, sizeof(cutscene_entities));
}

void run_script() BANKED
{
	while (1) {
		#define NEXT_BYTE banked_get16(cur_script_bank, cur_script++)
		#define CURR_BYTE banked_get16(cur_script_bank, cur_script)
		switch (NEXT_BYTE) {
		case 0:
			return;
		case SCRIPT_LOAD_ENTITY: {
			uint8_t index = NEXT_BYTE;
			memset(&cutscene_entities[index], 0, sizeof(cutscene_entity));
			cutscene_entities[index].bank = NEXT_BYTE;
			cutscene_entities[index].data = (entity_data*) NEXT_BYTE;
			break;
		}
		case SCRIPT_JUMP: {
			cur_script = (int*) CURR_BYTE;
			break;
		}
		case SCRIPT_POSITION_ENTITY: {
			uint8_t index = NEXT_BYTE;
			cutscene_entities[index].x = NEXT_BYTE;
			cutscene_entities[index].y = NEXT_BYTE;
			break;
		}
		case SCRIPT_MOVE_ENTITY: {
			uint8_t index = NEXT_BYTE;
			uint8_t x = NEXT_BYTE;
			uint8_t y = NEXT_BYTE;
			script_return = false;
			if (cutscene_entities[index].y > y) {
				cutscene_entities[index].y--;
			} else if (cutscene_entities[index].y < y) {
				cutscene_entities[index].y++;
			} else if (cutscene_entities[index].x > x) {
				cutscene_entities[index].x--;
			} else if (cutscene_entities[index].x < x) {
				cutscene_entities[index].x++;
			} else {
				script_return = true;
			}
			break;
		}
		case SCRIPT_SET_ENTITY_DIR: {
			uint8_t index = NEXT_BYTE;
			cutscene_entities[index].dir = NEXT_BYTE;
			break;
		}
		case SCRIPT_SET_ENTITY_FRAME: {
			uint8_t index = NEXT_BYTE;
			cutscene_entities[index].frame = NEXT_BYTE;
			break;
		}
		case SCRIPT_RENDER_ENTITY: {
			uint8_t index = NEXT_BYTE;
			draw_static_entity(cutscene_entities[index].data, cutscene_entities[index].bank,
					   cutscene_entities[index].dir, cutscene_entities[index].frame,
					   (char*) (0x8000 + index * 64), index);
			break;
		}
		case SCRIPT_ANIM_ENTITY: {
			uint8_t index = CURR_BYTE;
			cutscene_entity* cur_entity = &cutscene_entities[NEXT_BYTE];
			if (!cur_entity->anim_timer) {
				cur_entity->anim_timer = NEXT_BYTE;
				if (cur_entity->frame == CURR_BYTE) {
					cur_script++;
					cur_entity->frame = NEXT_BYTE;
				} else {
					cur_entity->frame = NEXT_BYTE;
					cur_script++;
				}
				draw_static_entity(cur_entity->data, cur_entity->bank,
						cur_entity->dir, cur_entity->frame,
						(char*) (0x8000 + index * 64), index);
			} else {
				cur_entity->anim_timer--;
				cur_script += 3;
			}
			break;
		}
		case SCRIPT_JUMP_IF_TRUE: {
			if (script_return) {
				cur_script = (int*) CURR_BYTE;
			}
			break;
		}
		case SCRIPT_SAY: {
			dialogue_bank = cur_script_bank;
			dialogue_string = (char*) NEXT_BYTE;
			print_dialogue();
			break;
		}
		case SCRIPT_SET: {
			*(char*) NEXT_BYTE = (char) NEXT_BYTE;
			break;
		}
		}
		#undef NEXT_BYTE
		#undef CURR_BYTE
	}
}

void simulate_cutscene() BANKED
{
	// Run the script until hitting a yield.
	run_script();

	// Render entities.
	reset_oam();
	char* oam_ptr = (char*) &shadow_OAM;
	for (uint8_t i = 0; i < NB_CUTSCENE_ENTITIES; i++) {
		if (cutscene_entities[i].data) {
			*oam_ptr++ = cutscene_entities[i].y + 16;
			*oam_ptr++ = cutscene_entities[i].x + 8;
			*oam_ptr++ = i * 4;
			*oam_ptr++ = i;		
			*oam_ptr++ = cutscene_entities[i].y + 16;
			*oam_ptr++ = cutscene_entities[i].x + 16;
			*oam_ptr++ = i* 4 + 2;
			*oam_ptr++ = i;
		}
	}
}