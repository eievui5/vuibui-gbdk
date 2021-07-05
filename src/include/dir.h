#pragma once

#define TURN_LEFT(dir) (((dir) - 1) & 0b11)
#define TURN_RIGHT(dir) (((dir) + 1) & 0b11)
#define FLIP(dir) (((dir) + 2) & 0b11)

enum dir {
	DIR_UP,
	DIR_RIGHT,
	DIR_DOWN,
	DIR_LEFT
};