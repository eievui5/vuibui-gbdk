#pragma bank 255

#include "include/move.h"

const void __at(255) __bank_lunge;

const move_data lunge_move = {
	.name = "Lunge",
	.power = 15,
	.range = 2,
};