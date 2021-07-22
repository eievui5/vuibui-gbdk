#pragma once

#include <stdint.h>

extern struct entity;

typedef struct {
	const char *name;
	const uint8_t power;
	const uint8_t range;
} move_data;

typedef struct {
	const move_data *data;
	uint8_t bank;
} move;