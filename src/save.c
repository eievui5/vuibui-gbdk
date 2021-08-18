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
	if (!strcmp(test_string, sram_corruption_check)) {
		strcpy(sram_corruption_check, test_string);
		return true;
	}
	return false;
}

bool get_sram_flag(uint8_t id) BANKED
{
	return active_save_file.flags[id >> 3] & (1 << (id & 0b111));
}

void set_sram_flag(uint8_t id) BANKED
{
	active_save_file.flags[id >> 3] |= (1 << (id & 0b111));
}