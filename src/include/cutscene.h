#pragma once

#include <gb/gb.h>
#include <stdint.h>

enum ScriptBytecode {
	SCRIPT_YIELD, // 2 bytes.
	SCRIPT_LOAD_ENTITY, // 10 bytes.
	SCRIPT_JUMP, // 6 bytes.
	SCRIPT_MOVE_ENTITY, // 8 bytes.
	SCRIPT_POSITION_ENTITY, // 8 bytes.
	SCRIPT_SET_ENTITY_DIR, // 6 bytes.
	SCRIPT_SET_ENTITY_FRAME, // 6 bytes.
	SCRIPT_RENDER_ENTITY, // 4 bytes.
	SCRIPT_ANIM_ENTITY, // 10 bytes.
	SCRIPT_JUMP_IF_TRUE, // 4 bytes.
	SCRIPT_SAY, // 4 bytes.
	SCRIPT_SET, // 6 bytes.
};

#define SCRIPT(name) const int name[] =
#define EXTERN_SCRIPT(name) extern const int name[];
#define YIELD() SCRIPT_YIELD,
#define LOAD_ENTITY(index, bank, src) SCRIPT_LOAD_ENTITY, index, bank, (int) src,
#define JUMP(dest) SCRIPT_JUMP, (int) &dest,
// Move until reaching a given location. Returns true if the location was reached.
#define MOVE_ENTITY(index, x, y) SCRIPT_MOVE_ENTITY, index, x, y,
#define SET_ENTITY_POS(index, x, y) SCRIPT_POSITION_ENTITY, index, x, y,
#define SET_ENTITY_DIR(index, dir) SCRIPT_SET_ENTITY_DIR, index, dir,
#define SET_ENTITY_FRAME(index, frame) SCRIPT_SET_ENTITY_FRAME, index, frame,
#define RENDER_ENTITY(index) SCRIPT_RENDER_ENTITY, index,
#define ANIMATE_ENTITY(index, mask, frame0, frame1) SCRIPT_ANIM_ENTITY, index, mask, frame0, frame1,
#define JUMP_IF_TRUE(dest) SCRIPT_JUMP_IF_TRUE, (int) &dest,
#define SAY(str) SCRIPT_SAY, (int) &(str),
#define SET(dest, val) SCRIPT_SET, (int) &(dest), val,

extern int* cur_script;
extern uint8_t cur_script_bank;
extern int script_return;

void init_cutscene() BANKED;
void simulate_cutscene() BANKED;