#pragma once

#include <gb/incbin.h>

#include "include/map.h"

INCBIN_EXTERN(field_tileset)

extern const void __at(255) __bank_field_mapdata;
extern const mapdata field_mapdata;