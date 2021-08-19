#pragma once

#include "include/bank.h"
#include "include/world.h"

INCBIN_EXTERN(crater_graphics)
INCBIN_EXTERN(crater_map)
INCBIN_EXTERN(crater_attr)
BANK_EXTERN(crater)

extern const map_node crater_house;
extern const map_node crater_woodland;
extern const map_node crater_fields;

extern const world_map crater_worldmap;