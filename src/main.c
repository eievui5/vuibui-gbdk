#include <gb/gb.h>
#include <gb/hardware.h>
#include <stdbool.h>
#include <stdint.h>

#include "map.h"
#include "include/hardware_flags.h"
#include "include/vector.h"

#define camera_speed 1

const unsigned char tiles[] = 
{
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        
        0xF0, 0x0F, 0xF0, 0x0F, 0xF0, 0x0F, 0xF0, 0x0F, 
        0xF0, 0x0F, 0xF0, 0x0F, 0xF0, 0x0F, 0xF0, 0x0F,
};

const unsigned char map[] = 
{
        0x00, 0x01, 0x01,
        0x01, 0x02, 0x01,
        0x01, 0x01, 0x00,
};


uint8_t cur_keys = 0, last_keys, new_keys, released_keys;

void main(void) 
{
        // Set up some initial tiles
        set_bkg_data(1, 2, tiles);
        set_bkg_tiles(0, 0, 3, 3, map);

        // Enable the Screen and Background
        LCDC_REG = LCDC_ENABLE | LCDC_BG_ENABLE;

        while (true) 
        {
                last_keys = cur_keys;
                cur_keys = joypad();
                new_keys = cur_keys & ~last_keys;
                released_keys = last_keys & ~cur_keys;

                if (cur_keys & J_DOWN) { camera_position.y += camera_speed; }
                if (cur_keys & J_UP) { camera_position.y -= camera_speed; }
                if (cur_keys & J_RIGHT) { camera_position.x += camera_speed; }
                if (cur_keys & J_LEFT) { camera_position.x -= camera_speed; }

                SCX_REG = camera_position.x; 
                SCY_REG = camera_position.y; 

                wait_vbl_done();

                scroll_map();
        }
}