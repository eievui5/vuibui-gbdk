#pragma once

#define FLIP(dir) (((dir) + 2) & 0b11)

enum dir {
	DIR_UP,
	DIR_RIGHT,
	DIR_DOWN,
	DIR_LEFT
};