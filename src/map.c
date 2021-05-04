#include <stdio.h>

#include "include/vector.h"

vec16 camera_position = { 0, 0 };

// The maximum map position is 8192, or 65536/8
vec16 map_position, last_map_position = { 0, 0 };

// Scroll the current map according to `camera_position`
void scroll_map(void)
{
        // Divide camera positon by 8 to get tile position
        map_position.x = camera_position.x >> 3;
        map_position.y = camera_position.y >> 3;

        // If the map position has updated load new tiles.
        if (map_position.x != last_map_position.x) 
        {
                // Check which direction the map has moved in.
                if (map_position.x < last_map_position.x) 
                {
                        printf("Updating col %u %u\n",
                               map_position.x & 0b11111, map_position.x);
                        
                }
                // When moving right, add an offset so that tiles show up past
                // the right window boundry.
                else
                {
                        printf("Updating col %u %u\n",
                               (map_position.x & 0b11111) + 19 , map_position.x + 19);
                }

                // Cache the map position so that no unneccessary updates occur.
                last_map_position.x = map_position.x;
        }
        if (map_position.y != last_map_position.y) 
        {
                // Check which direction the map has moved in.
                if (map_position.y < last_map_position.x)
                {
                        printf("Updating row %u %u\n",
                               map_position.y & 0b11111, map_position.y);
                }
                // When moving down, add an offset so that tiles show up past
                // the bottom window boundry.
                else
                {
                        printf("Updating row %u %u\n",
                               (map_position.y & 0b11111) + 17, map_position.y + 17);
                }

                // Cache the map position so that no unneccessary updates occur.
                last_map_position.y = map_position.y;
        }

}