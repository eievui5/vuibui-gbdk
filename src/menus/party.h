#pragma once

#include <stdint.h>

enum party_type {
	PARTY_NULL = 0,
	PARTY_HEALTH = 1,
	PARTY_FATIGUE = 2,
	PARTY_LEVEL = 4
};

void draw_party_entity(uint8_t i, uint8_t dir, uint8_t frame) NONBANKED;
void draw_party(uint8_t x, uint8_t y, uint8_t font_tile, uint8_t spr_x, 
	uint8_t spr_y, uint8_t spacing, uint8_t type) BANKED;
void party_menu();