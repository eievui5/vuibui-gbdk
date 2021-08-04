#pragma once

#include <gb/gb.h>
#include <stdint.h>

typedef struct {
	uint8_t x;
	uint8_t y;
} uvec8;

typedef struct {
	int8_t x;
	int8_t y;
} vec8;

typedef struct {
	uint16_t x;
	uint16_t y;
} uvec16;

typedef struct {
	int16_t x;
	int16_t y;
} vec16;

void move_direction(vec8 *vec, uint8_t dir) BANKED;
uint8_t get_direction(int8_t x, int8_t y) BANKED;