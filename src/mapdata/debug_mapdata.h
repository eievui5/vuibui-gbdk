#pragma once

#include "include/map.h"

extern const void __at(255) __bank_debug_mapdata;
#define bank_debug_mapdata ((unsigned char)&__bank_debug_mapdata)

extern const mapdata debug_mapdata;