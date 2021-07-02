#pragma bank 255

#include <gb/gb.h>
#include <rand.h>
#include <stdbool.h>
#include <string.h>
#include "include/dir.h"
#include "include/int.h"
#include "include/map.h"
#include "include/vec.h"

u8 map[64][64];
u16 camera_x = 0;
u16 camera_y = 0;
u8 current_mapdata_bank;
mapdata *current_mapdata;

void update_camera(u16 x, u16 y) NONBANKED
{
	static u16 last_camera_x = 0;
	static u16 last_camera_y = 0;
	static u8 last_tile_x = 0;
	static u8 last_tile_y = 0;

	if (x > 32 * 64)
		camera_x = 0;
	else if (x > 16 * 54)
		camera_x = 16 * 54;
	else
		camera_x = x;
	if (y > 32 * 64)
		camera_y = 0;
	else if (y > 16 * 57)
		camera_y = 16 * 57;
	else
		camera_y = y;
	u8 cur_tile_x = camera_x >> 3;
	u8 cur_tile_y = camera_y >> 3;

	if (cur_tile_x != last_tile_x) {
		u8 tmpb = _current_bank;
		SWITCH_ROM_MBC1(current_mapdata_bank);

		u8 ptrx = cur_tile_x + (camera_x > last_camera_x ? 20 : 0);
		if (ptrx > 127)
			ptrx = 127;
		u8 ptry = cur_tile_y < 113 ? cur_tile_y : 113;
		for (u8 i = 0; i < 15; i++) {
			set_vram_byte(
				(void *)(0x9800 + ptrx % 32 + (ptry % 32) * 32),
				current_mapdata->metatiles[
					map[ptry >> 1][ptrx >> 1]
				].tiles[ptrx & 1 + (ptry & 1) * 2]
			);
			ptry++;
		}
		SWITCH_ROM_MBC1(tmpb);
	}
	if (cur_tile_y != last_tile_y) {
		u8 tmpb = _current_bank;
		SWITCH_ROM_MBC1(current_mapdata_bank);

		u8 ptrx = cur_tile_x < 127 ? cur_tile_x : 127;
		u8 ptry = cur_tile_y + (camera_y > last_camera_y ? 14 : 0);
		if (ptry > 127)
			ptry = 127;
		for (u8 i = 0; i < 21; i++) {
			set_vram_byte(
				(void *)(0x9800 + ptrx % 32 + (ptry % 32) * 32),
				current_mapdata->metatiles[
					map[ptry >> 1][ptrx >> 1]
				].tiles[ptrx & 1 + (ptry & 1) * 2]
			);
			ptrx++;
		}
		SWITCH_ROM_MBC1(tmpb);
	}
	last_tile_x = cur_tile_x;
	last_tile_y = cur_tile_y;
	last_camera_x = camera_x;
	last_camera_y = camera_y;
}

// Return the collision of the tile at (`x`, `y`).
u8 get_collision(u16 x, u16 y) NONBANKED
{
	u8 tmpb = _current_bank;
	SWITCH_ROM_MBC1(current_mapdata_bank);

	u8 rtrn = current_mapdata->metatiles[map[y][x]].collision;

	SWITCH_ROM_MBC1(tmpb);
	return rtrn;
}

void load_mapdata(mapdata *data, u8 bank) NONBANKED
{
	u8 tmpb = _current_bank;
	current_mapdata_bank = bank;
	SWITCH_ROM_MBC1(bank);

	current_mapdata = data;
	set_bkg_data(0, 128, data->tileset);

	SWITCH_ROM_MBC1(tmpb);
}

void force_render_map() NONBANKED
{
	u8 tmpb = _current_bank;
	SWITCH_ROM_MBC1(current_mapdata_bank);

	u8 x = camera_x >> 3;
	for (u8 i = 0; i < 21; i++) {
		u8 y = camera_y >> 3;
		for (u8 j = 0; j < 19; j++) {
			set_vram_byte(
				(void *)(0x9800 + x % 32 + (y % 32) * 32),
				current_mapdata->metatiles[
					map[y >> 1][x >> 1]
				].tiles[x & 1 + (y & 1) * 2]
			);
			y++;
		}
		x++;
	}
	
	SWITCH_ROM_MBC1(tmpb);
}

// Force-draw walls around the outer edges of the map.
void force_walls() BANKED
{
	memset(&map[0][0], 1, 64);
	for (u8 i = 1; i < 63; i++) {
		map[i][0] = 1;
		map[i][63] = 1;
	}
	memset(&map[63][0], 1, 64);
}

/**
 * Contain the cursor within the map's bounds.
 * 
 * @param cur	Pointer to cursor.
 * 
 * @returns 	Boolean; whether or not the cursor was adjusted.
*/
bool contain_cursor(uvec8 *cur) BANKED
{
	if (cur->x == 255) {
		cur->x = 0;
		return true;
	} else if (cur->x == 64) {
		cur->x = 63;
		return true;
	} else if (cur->y == 255) {
		cur->y = 0;
		return true;
	} else if (cur->y == 64) {
		cur->y = 63;
		return true;
	}
	return false;
}

/**
 * Place tiles in a line. Starts at the cursor's location and returns its
 * stopping place.
 * 
 * @param cur	Pointer to cursor.
 * @param tile	Tile type to place.
 * @param dir	Direction to draw.
 * @param len	Number of tiles to draw.
*/
void map_walk(uvec8 *cur, u8 tile, u8 dir, u8 len) BANKED
{
	for (u8 i = 0; i < len; i++) {
		u8 sw_dir = dir;
		if (!(rand() & 0b111))
			sw_dir = rand() & 0b11;
		switch (sw_dir) {
		case DIR_UP:
			cur->y--;
			break;
		case DIR_RIGHT:
			cur->x++;
			break;
		case DIR_DOWN:
			cur->y++;
			break;
		case DIR_LEFT:
			cur->x--;
			break;
		}
		if (contain_cursor(cur))
			return;
		map[cur->y][cur->x] = tile;
	}
}

/**
 * Generate a room of a given size, centered on the cursor. The room will be
 * offset if too far to the top/left, and squashed if too far to the bottom/
 * right.
 * 
 * @param cur		Pointer to cursor.
 * @param width 	Width of the room.
 * @param height	Height of the room.
 * 
 * @returns 		Cursor at a random edge of the room.
*/
void generate_room(uvec8 *cur, u8 width, u8 height) BANKED
{
	cur->x -= width/2;
	cur->y -= height/2 + 1;

	for (u8 y = height; y; y--) {
		cur->y++;
		for (u8 x = width; x; x--) {
			contain_cursor(cur);
			map[cur->y][cur->x] = 0;
			cur->x++;
		}
		cur->x -= width;
	}

	// We left off at the bottom-left corner. Offset to an exit position.
	switch (rand() & 0b11) {
	case DIR_DOWN:
		// Only offset X when going down...
		cur->x += rand() % width;
		break;
	case DIR_LEFT:
		// ... Y when going left...
		cur->y -= rand() % height;
		break;
	case DIR_RIGHT:
		// ... and both for right and up.
		cur->x += width;
		cur->y -= rand() % height;
		break;
	case DIR_UP:
		cur->x += rand() % width;
		cur->y -= height;
		break;
	}
	contain_cursor(cur);
}

/**
 * Attempts to generate an exit. Will search for a 3x3 area of clear tiles to
 * place the exit tile on.
*/
void generate_exit() BANKED
{
	while (1) {
		u8 x = rand() & 0b111111;
		u8 y = rand() & 0b111111;
		if (!(
			map[y][x]     || map[y][x + 1]	   || map[y][x - 1] ||
			map[y + 1][x] || map[y + 1][x + 1] || map[y + 1][x - 1] ||
			map[y - 1][x] || map[y - 1][x + 1] || map[y - 1][x - 1]
		)) {
			map[y][x] = EXIT_COLL;
			return;
		}
	}
}

// Temporary function to test map generation.
void generate_map() BANKED
{
	memset(map, 1, sizeof(map));
	map[32][32] = 0;
	uvec8 cur = {32, 32};
	for (u8 i = 0; i < 8; i++) {
		map_walk(&cur, 0, rand() & 0b11, (rand() & 0b1111) + 4);
		map_walk(&cur, 0, rand() & 0b11, (rand() & 0b1111) + 4);
		map_walk(&cur, 0, rand() & 0b11, (rand() & 0b1111) + 4);
		generate_room(&cur, (rand() & 0b11) + 4, (rand() & 0b11) + 4);
	}
	force_walls();
	generate_exit();
}