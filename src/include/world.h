#pragma once

#include <stdint.h>

#include "include/save.h"

typedef enum {DUNGEON_NODE, FORK_NODE, } NodeType;

typedef struct {
	uint8_t x;
	uint8_t y;
	NodeType type;
	void *connections[4];
	void *level;
	uint8_t bank;
	// A node can only be selected if this flag is set.
	// If this flag is 0, the node can always be selected.
	enum SramFlag unlock_flag;
} map_node;

typedef struct {
	const char *name;
	const map_node *nodes[];
} world_map;

extern map_node *current_mapnode;

void init_worldmap() NONBANKED;
void simulate_worldmap() NONBANKED;