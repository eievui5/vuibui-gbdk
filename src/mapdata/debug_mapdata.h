#pragma once

#include <gb/incbin.h>

#include "include/map.h"

INCBIN_EXTERN(debug_tileset)

extern const void __at(2) __bank_debug_mapdata;

extern const mapdata debug_mapdata;