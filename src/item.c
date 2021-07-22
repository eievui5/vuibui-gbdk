#include <rand.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "include/entity.h"
#include "include/hud.h"
#include "include/item.h"
#include "include/map.h"
#include "include/rendering.h"

const char a_infinitive[] = "a";
const char an_infinitive[] = "an";
const char pickup_message[] = "%s picked up %s %s.";
const char inventory_full[] = "%s's inventory is full.";

// Array of items lying in the world
world_item world_items[NB_WORLD_ITEMS];
item inventory[INVENTORY_SIZE];

// Searches for a valid space to create an item in the inventory, returning
// whether or not it succeeded.
bool add_item(const item_data *data, const uint8_t bank)
{
	for (uint8_t i = 0; i < INVENTORY_SIZE; i++) {
		if (!inventory[i]) {
			inventory[i].data = data;
			inventory[i].bank = bank;
			return true;
		}
	}
	return false;
}

// Attempt to pick up an item given an index. Constructs a message and searches
// for space in the invetory.
void pickup_item(uint8_t i)
{

	char buffer[MESSAGE_SIZE];
	if (add_item(world_items[i].data, world_items[i].bank)) {
		uint8_t temp_bank = _current_bank;
		SWITCH_ROM_MBC1(current_mapdata_bank);
		vec8 tile_pos = {world_items[i].x * 2, world_items[i].y * 2};
		draw_tile(tile_pos.x, tile_pos.y);
		draw_tile(tile_pos.x + 1, tile_pos.y);
		draw_tile(tile_pos.x, tile_pos.y + 1);
		draw_tile(tile_pos.x + 1, tile_pos.y + 1);

		SWITCH_ROM_MBC1(world_items[i].bank);
		sprintf(
			buffer,
			pickup_message,
			PLAYER.name,
			world_items[i].data->name[0] == 'A' ? \
				an_infinitive : a_infinitive,
			world_items[i].data->name
		);

		SWITCH_ROM_MBC1(temp_bank);
		memset(&world_items[i], 0, sizeof(world_item));
	} else
		sprintf(buffer, inventory_full, PLAYER.name);

	print_hud(buffer);

}

// Load the item graphics for each of the current items into the last 16 tiles
// of VRAM.
void load_item_graphics()
{
	for (uint8_t i = 0; i < NB_WORLD_ITEMS; i++) {
		if (!world_items[i].data)
			continue;
		SWITCH_ROM_MBC1(world_items[i].bank);
		vmemcpy((void *)(0x9700 + i * 64), 64, world_items[i].data->graphic);
	}
}

void generate_items()
{
	uint8_t temp_bank = _current_bank;
	SWITCH_ROM_MBC1(current_mapdata_bank);
	for (uint8_t i = 0; i < 4; i++) {
		uint8_t index = rand();
		uint8_t item_index = 0;
		while (current_mapdata->item_table[item_index].weight < index)
			item_index++;
		if (!current_mapdata->item_table[item_index].ptr)
			continue;
		while (1) {
			uint8_t x = rand() & 0b111111;
			uint8_t y = rand() & 0b111111;
			if (!(get_collision(x, y) | 
			    get_collision(x + 1, y) |
			    get_collision(x - 1, y) | 
			    get_collision(x, y + 1) |
			    get_collision(x + 1, y + 1) | 
			    get_collision(x - 1, y + 1) |
			    get_collision(x, y - 1) | 
			    get_collision(x - 1, y - 1) |
			    get_collision(x - 1, y - 1))) {
				world_items[i].data = current_mapdata->item_table[item_index].ptr;
				world_items[i].bank = current_mapdata->item_table[item_index].bank;
				world_items[i].x = x;
				world_items[i].y = y;
				break;
			}
		}
	}
	SWITCH_ROM_MBC1(temp_bank);
}