typedef enum {
	CLEAR_COLLISION,
	WALL_COLLISION
} COLLISION;

COLLISION get_collision(uint16_t x, uint16_t y);