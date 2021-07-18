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
bool add_item(const item_data *data, const u8 bank)
{
	for (u8 i = 0; i < INVENTORY_SIZE; i++) {
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
void pickup_item(u8 i)
{

	char buffer[MESSAGE_SIZE];
	if (add_item(world_items[i].data, world_items[i].bank)) {
		u8 temp_bank = _current_bank;
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
	for (u8 i = 0; i < NB_WORLD_ITEMS; i++) {
		if (!world_items[i].data)
			continue;
		SWITCH_ROM_MBC1(world_items[i].bank);
		vmemcpy((void *)(0x9700 + i * 64), 64, world_items[i].data->graphic);
	}
}
