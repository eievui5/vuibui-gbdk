#pragma once

#include "include/int.h"

extern struct entity;

typedef struct {
	const char *name;
	const u8 power;
	const u8 range;
} move_data;

typedef struct {
	const move_data *data;
	u8 bank;
} move;