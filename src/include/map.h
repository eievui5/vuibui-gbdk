#include "include/int.h"

// Y, X order.
extern u8 map[64][64];
extern u16 camera_x;
extern u16 camera_y;

void update_camera(u16 x, u16 y);
void generate_map();