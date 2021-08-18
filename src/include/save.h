#pragma once

#include <gb/gb.h>

enum SramFlag {
	FLAG_NULL = 0, // Define a null flag.
	// Level completion flags.
		// World 1 (Crater)
			FLAG_WORLD_1_1,
	MAX_FLAGS
};

typedef struct {
	unsigned char flags[MAX_FLAGS / 8 + 1];
} save_file;

bool test_sram_corruption() BANKED;
bool get_sram_flag(uint8_t id) BANKED;
void set_sram_flag(uint8_t id) BANKED;