#pragma once

#include "include/int.h"

#define NB_WORLD_ITEMS 4
#define INVENTORY_SIZE 5

enum item_type {
	NULL_ITEM,
	HEAL_ITEM,
};

// Used to extend the item_data struct for polymorphism.

typedef struct {
	const char *name;
	const char *graphic;
	const u8 palette;
	const char *desc;
	const u8 type;
} item_data;

// Item classes. Cast item_data to these based on type.
typedef struct {
	const item_data data;
	const u8 health;
	const u8 fatigue;
} healitem_data;

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
extern item inventory[INVENTORY_SIZE];

void pickup_item(u8 i);
void load_item_graphics();