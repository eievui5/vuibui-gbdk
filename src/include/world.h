#pragma once

#include <stdint.h>

typedef enum {DUNGEON_NODE, FORK_NODE, } NodeType;

typedef struct {
	uint8_t x;
	uint8_t y;
	NodeType type;
	void *connections[4];
	void *level;
	uint8_t bank;
} map_node;

extern map_node *current_mapnode;

uint8_t simulate_worldmap() BANKED;