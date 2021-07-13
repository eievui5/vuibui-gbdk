#pragma once

#include "include/int.h"

#define NB_WORLD_ITEMS 4

typedef struct {
	const char *name;
	const char *graphic;
	const u8 palette;
} item_data;

typedef struct {
	const item_data *data;
	u8 bank;
} item;

typedef struct {
	const item_data *data;
	u8 bank;
	u8 x;
	u8 y;
	u8 palette;
} world_item;

extern world_item world_items[NB_WORLD_ITEMS];

void pickup_item(u8 i);