#pragma once

#include <stdint.h>

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
	const uint8_t palette;
	const char *desc;
	short *colors;
	const uint8_t type;
} item_data;

// Item classes. Cast item_data to these based on type.
typedef struct {
	const item_data data;
	const uint8_t health;
	const uint8_t fatigue;
} healitem_data;

typedef struct {
	const item_data *data;
	uint8_t bank;
} item;

typedef struct {
	const item_data *data;
	uint8_t bank;
	uint8_t x;
	uint8_t y;
	uint8_t palette;
} world_item;

extern world_item world_items[NB_WORLD_ITEMS];
extern item inventory[INVENTORY_SIZE];

void pickup_item(uint8_t i) BANKED;
void load_item_graphics() NONBANKED;
void generate_items() NONBANKED;