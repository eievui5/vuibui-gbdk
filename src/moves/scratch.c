#pragma bank 255

#include "include/move.h"

const void __at(255) __bank_scratch;

const move_data scratch_move = {
	.name = "Scratch",
	.power = 1,
	.range = 1,
};