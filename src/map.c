#include <string.h>
#include "include/int.h"

u8 map[64][64];
u16 camera_x = 0;
u16 camera_y = 0;

void update_camera(u16 x, u16 y) {
	if (x > 16 * 64) {
		camera_x = 16 * 64;
	} else if (x > 32 * 64) {
		camera_x = 0;
	} else {
		camera_x = x;
	}
	if (y > 16 * 64) {
		camera_y = 16 * 64;
	} else if (y > 32 * 64) {
		camera_y = 0;
	} else {
		camera_y = y;
	}
}

void generate_map()
{
	memset(&map[0][0], 0xFF, 64);
	for (u8 i = 1; i < 63; i++) {
		map[i][0] = 0xFF;
		map[i][63] = 0xFF;
	}
	memset(&map[63][0], 0xFF, 64);
}