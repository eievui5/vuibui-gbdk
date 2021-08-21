#pragma bank 255

#include <gb/gb.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "include/save.h"

extern const char test_string[];
extern char sram_corruption_check[];

save_file active_save_file;

bool test_sram_corruption() BANKED
{
	ENABLE_RAM_MBC5;
	if (strcmp(test_string, sram_corruption_check) != 0) {
		strcpy(sram_corruption_check, test_string);
		DISABLE_RAM_MBC5;
		return true;
	}
	DISABLE_RAM_MBC5;
	return false;
}

void save_entity(entity_save *dest, entity *src) BANKED
{
	strcpy(dest->name, src->name);
	memcpy(dest->moves, src->moves, sizeof(move) * 4);
}

void save_data() BANKED
{
	// Construct the save file.
	ENABLE_RAM_MBC5;
	memcpy(&sram_file_1, &active_save_file, sizeof(save_file));
	DISABLE_RAM_MBC5;
}

bool get_sram_flag(uint8_t id) BANKED
{
	return active_save_file.flags[id >> 3] & (1 << (id & 0b111));
}

void set_sram_flag(uint8_t id) BANKED
{
	active_save_file.flags[id >> 3] |= (1 << (id & 0b111));
}