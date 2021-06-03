#pragma once

#include <stdint.h>

#include "entity.h"

#define END_METASPRITE -128

extern uint8_t oam_index;

void render_entity(entity *target, uint8_t x_off, uint8_t y_off);
void render_movement(entity *target, uint8_t progress);