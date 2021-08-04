#pragma bank 255

#include <stdlib.h>

#include "include/dir.h"
#include "include/vec.h"

/**
 * Offset a vector (a pointer of course... thanks SDCC...) based on a direction
 * value.
 *
 * @param vec	Pointer to the target vector.
 * @param dir	The direction to use as an offset.
*/
void move_direction(vec8 *vec, uint8_t dir) BANKED
{
	switch (dir) {
	case DIR_UP:
		vec->y--;
		break;
	case DIR_RIGHT:
		vec->x++;
		break;
	case DIR_DOWN:
		vec->y++;
		break;
	case DIR_LEFT:
		vec->x--;
		break;
	}
}

/**
 * Return the direction of a given vector. Prefers X over Y, and returns DIR_UP
 * for 0, 0.
 *
 * @param x	X direction.
 * @param y	Y direction.
 *
 * @return	Resulting direction value.
*/
uint8_t get_direction(int8_t x, int8_t y) BANKED
{
	if (abs(x) > abs(y)) {
		if (x > 0)
			return DIR_RIGHT;
		else
			return DIR_LEFT;
	}
	if (y > 0)
		return DIR_DOWN;
	return DIR_UP;
}