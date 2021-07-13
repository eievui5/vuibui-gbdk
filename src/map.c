#pragma bank 255

#include <gb/cgb.h>
#include <gb/gb.h>
#include <rand.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "include/dir.h"
#include "include/int.h"
#include "include/hud.h"
#include "include/item.h"
#include "include/map.h"
#include "include/rendering.h"
#include "include/vec.h"

u8 map[64][64];
uvec16 camera = {0, 0};
u8 current_mapdata_bank;
mapdata *current_mapdata;

void draw_tile(u8 x, u8 y) NONBANKED
{
	u16 tile_ptr = 0x9800 + (x & 0b11111) + ((y & 0b11111) << 5);
	vset(
		tile_ptr,
		current_mapdata->metatiles[
			map[y >> 1][x >> 1]
		].tiles[(x & 1) + (y & 1) * 2]
	);
	if (_cpu == CGB_TYPE) {
		VBK_REG = 1;
		vset(
			tile_ptr,
			current_mapdata->metatiles[
				map[y >> 1][x >> 1]
			].attrs[(x & 1) + (y & 1) * 2]
		);
		VBK_REG = 0;
	}
}

void render_item(u8 i) NONBANKED
{
	u8 temp_bank = _current_bank;
	SWITCH_ROM_MBC1(world_items[i].bank);

	const item_data *self = world_items[i].data;
	u16 tile_ptr = 0x9800 + ((world_items[i].x * 2) & 0b11111) + (world_items[i].y * 2 & 0b11111) * 32;
	u8 tile_id = 0x70 + i * 4;
	vset(tile_ptr, tile_id++);
	vset(tile_ptr + 1, tile_id++);
	vset(tile_ptr + 32, tile_id++);
	vset(tile_ptr + 33, tile_id++);
	if (_cpu == CGB_TYPE) {
		VBK_REG = 1;
		u8 tile_attr = world_items[i].palette;
		tile_ptr;
		vset(tile_ptr, tile_attr);
		vset(tile_ptr + 1, tile_attr);
		vset(tile_ptr + 32, tile_attr);
		vset(tile_ptr + 33, tile_attr);
		VBK_REG = 0;
	}

	SWITCH_ROM_MBC1(temp_bank);
}

void update_camera(u16 x, u16 y) NONBANKED
{
	static u16 last_camera_x = 0;
	static u16 last_camera_y = 0;
	static u8 last_tile_x = 0;
	static u8 last_tile_y = 0;

	if (x > 32 * 64)
		camera.x = 0;
	else if (x > 16 * 54)
		camera.x = 16 * 54;
	else
		camera.x = x;
	if (y > 32 * 64)
		camera.y = 0;
	else if (y > 16 * 57)
		camera.y = 16 * 57;
	else
		camera.y = y;
	u8 cur_tile_x = camera.x >> 3;
	u8 cur_tile_y = camera.y >> 3;

	if (cur_tile_x != last_tile_x) {
		u8 tmpb = _current_bank;
		SWITCH_ROM_MBC1(current_mapdata_bank);

		u8 ptrx = cur_tile_x + (camera.x > last_camera_x ? 20 : 0);
		if (ptrx > 127)
			ptrx = 127;
		u8 ptry = cur_tile_y < 113 ? cur_tile_y : 113;
		u8 i = 0;
		for (; i < 15; i++) {
			draw_tile(ptrx, ptry);
			ptry++;
		}
		for (i = 0; i < NB_WORLD_ITEMS; i++)
			if (world_items[i].data)
				if (
					world_items[i].x == ptrx / 2 &&
					world_items[i].y < ptry / 2 &&
					world_items[i].y > (ptry - 17) / 2
				)
					render_item(i);
		SWITCH_ROM_MBC1(tmpb);
	}
	if (cur_tile_y != last_tile_y) {
		u8 tmpb = _current_bank;
		SWITCH_ROM_MBC1(current_mapdata_bank);

		u8 ptrx = cur_tile_x < 127 ? cur_tile_x : 127;
		u8 ptry = cur_tile_y + (camera.y > last_camera_y ? 14 : 0);
		if (ptry > 127)
			ptry = 127;
		u8 i = 0;
		for (; i < 21; i++) {
			draw_tile(ptrx, ptry);
			ptrx++;
		}
		for (i = 0; i < NB_WORLD_ITEMS; i++)
			if (world_items[i].data)
				if (
					world_items[i].y == ptry / 2 &&
					world_items[i].x < ptrx / 2 &&
					world_items[i].x > (ptrx - 23) / 2
				)
					render_item(i);
		SWITCH_ROM_MBC1(tmpb);
	}
	last_tile_x = cur_tile_x;
	last_tile_y = cur_tile_y;
	last_camera_x = camera.x;
	last_camera_y = camera.y;
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
	if (_cpu == CGB_TYPE)
		set_bkg_palette(0, 7, current_mapdata->colors);

	SWITCH_ROM_MBC1(tmpb);
}

void force_render_map() NONBANKED
{
	u8 tmpb = _current_bank;
	SWITCH_ROM_MBC1(current_mapdata_bank);

	u8 x = camera.x >> 3;
	for (u8 i = 0; i < 21; i++) {
		u8 y = camera.y >> 3;
		for (u8 j = 0; j < 19; j++) {
			draw_tile(x, y);
			for (u8 j = 0; j < NB_WORLD_ITEMS; j++)
				if (world_items[j].data)
					if (
						world_items[j].x == x / 2 &&
						world_items[j].y == y / 2
					)
						render_item(j);
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

/**
 * Column postprocessing. Only vertical tiles are considered when autotiling.
 * 0 - Top; 1 - Connection; 2 - Bottom; 3 - Standalone
*/
void column_postprocess() NONBANKED
{
	u8 tmpb = _current_bank;
	SWITCH_ROM_MBC1(current_mapdata_bank);

	for (u8 x = 0; x < 64; x++)
		if (map[1][x] != NO_COLL)
			map[0][x] = current_mapdata->wall_palette[1];
		else
			map[0][x] = current_mapdata->wall_palette[2];

	for (u8 y = 1; y < 63; y++) {
		for (u8 x = 0; x < 64; x++) {
			if (map[y][x] == NO_COLL)
				continue;
			else if (map[y - 1][x] != NO_COLL && map[y + 1][x] != NO_COLL)
				map[y][x] = current_mapdata->wall_palette[1];
			else if (map[y - 1][x] != NO_COLL)
				map[y][x] = current_mapdata->wall_palette[2];
			else if (map[y + 1][x] != NO_COLL)
				map[y][x] = current_mapdata->wall_palette[0];
			else
				map[y][x] = current_mapdata->wall_palette[3];
		}
	}

	for (u8 x = 0; x < 64; x++)
		if (map[62][x] != NO_COLL)
			map[63][x] = current_mapdata->wall_palette[1];
		else
			map[63][x] = current_mapdata->wall_palette[0];

	SWITCH_ROM_MBC1(tmpb);
}

// Temporary function to test map generation.
void generate_map() BANKED
{
	memset(map, 1, sizeof(map));
	map[32][32] = 0;
	uvec8 cur = {32, 32};
	generate_room(&cur, 9, 9);
	for (u8 i = 0; i < 8; i++) {
		map_walk(&cur, NO_COLL, rand() & 0b11, (rand() & 0b1111) + 4);
		map_walk(&cur, NO_COLL, rand() & 0b11, (rand() & 0b1111) + 4);
		map_walk(&cur, NO_COLL, rand() & 0b11, (rand() & 0b1111) + 4);
		generate_room(&cur, (rand() & 0b11) + 4, (rand() & 0b11) + 4);
	}
	force_walls();
	column_postprocess();
	//generate_exit();
}