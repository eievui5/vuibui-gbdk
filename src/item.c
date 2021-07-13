#include <stdio.h>
#include <string.h>

#include "include/entity.h"
#include "include/hud.h"
#include "include/item.h"
#include "include/map.h"

const char a_infinitive[] = "a";
const char an_infinitive[] = "an";
const char pickup_message[] = "%s picked up %s %s.";

// Array of items lying in the world
world_item world_items[NB_WORLD_ITEMS];

void pickup_item(u8 i)
{
	vec8 tile_pos = {world_items[i].x * 2, world_items[i].y * 2};
	draw_tile(tile_pos.x, tile_pos.y);
	draw_tile(tile_pos.x + 1, tile_pos.y);
	draw_tile(tile_pos.x, tile_pos.y + 1);
	draw_tile(tile_pos.x + 1, tile_pos.y + 1);

	char buffer[MESSAGE_SIZE];
	sprintf(
		buffer, 
		pickup_message, 
		PLAYER.name, 
		world_items[i].data->name[0] == 'A' ? an_infinitive : a_infinitive,
		world_items[i].data->name
	);
	print_hud(buffer);

	memset(&world_items[i], 0, sizeof(world_item));
}