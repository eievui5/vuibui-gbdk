#pragma once

#include <gb/gb.h>
#include <stdbool.h>
#include <stdint.h>

#include "include/entity.h"

enum SramFlag {
	FLAG_NULL = 0, // Define a null flag.
	// Level completion flags.
		// World 1 (Crater)
			FLAG_WORLD_1_1,
	MAX_FLAGS
};

// Saved entity data.
typedef struct {
	char name[ENTITY_NAME_LEN];
	move moves[4];
	uint8_t level;
	uint16_t xp;
} entity_save;

typedef struct {
	// A bitfield of boolean flags.
	unsigned char flags[MAX_FLAGS / 8 + 1];
	// Saved entity data for the player and various allies.
	entity_save player;
} save_file;

extern save_file sram_file_1;

bool test_sram_corruption() BANKED;
bool get_sram_flag(uint8_t id) BANKED;
void set_sram_flag(uint8_t id) BANKED;