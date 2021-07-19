#pragma once

enum party_type {
	PARTY_NULL = 0,
	PARTY_HEALTH = 1,
	PARTY_FATIGUE = 2,
};

void draw_party_entity(u8 i, u8 dir, u8 frame) NONBANKED;
void draw_party(u8 x, u8 y, u8 font_tile, u8 spr_x, u8 spr_y, u8 spacing, 
	u8 type) BANKED;
void party_menu();