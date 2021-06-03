#pragma once

#include <stdint.h>

#define DIR_DOWN 0
#define DIR_UP 1
#define DIR_RIGHT 2
#define DIR_LEFT 3

typedef enum {
	up,
	right,
	down,
	left,
} direction;

typedef struct { uint8_t x; uint8_t y; } vec8;

typedef struct { uint16_t x; uint16_t y; } vec16;