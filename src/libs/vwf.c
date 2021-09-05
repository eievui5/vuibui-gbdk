#include <gb/gb.h>
#include <stdint.h>
#include <string.h>

#include "vwf.h"

vwf_farptr_t vwf_fonts[4];

uint8_t vwf_current_offset;
static uint8_t vwf_tile_data[16 * 2];
uint8_t vwf_current_mask;
uint8_t vwf_current_rotate;
uint8_t vwf_inverse_map;
uint8_t vwf_current_tile;
uint8_t vwf_text_bkg_fill = 0;

font_desc_t vwf_current_font_desc;
uint8_t vwf_current_font_bank;

void vwf_print_shift_char(void * dest, const void * src, uint8_t bank);
void vwf_memcpy(void* to, const void* from, size_t n, uint8_t bank);
uint8_t vwf_read_banked_ubyte(const void * src, uint8_t bank) __preserves_regs(b, c);
void vwf_set_banked_bkg_data(uint8_t i, uint8_t l, const unsigned char* ptr, uint8_t bank);
void vwf_set_banked_win_data(uint8_t i, uint8_t l, const unsigned char* ptr, uint8_t bank);

void vwf_print_reset(uint8_t tile) {
	vwf_current_tile = tile;
	vwf_current_offset = 0;
	memset(vwf_tile_data, vwf_text_bkg_fill, sizeof(vwf_tile_data));
}

uint8_t vwf_print_render(const unsigned char ch) {
	uint8_t letter = vwf_read_banked_ubyte(vwf_current_font_desc.recode_table + (ch & ((vwf_current_font_desc.attr & RECODE_7BIT) ? 0x7fu : 0xffu)), vwf_current_font_bank);
	const uint8_t * bitmap = vwf_current_font_desc.bitmaps + letter * 16u;
	if (vwf_current_font_desc.attr & FONT_VWF) {
		vwf_inverse_map = (vwf_current_font_desc.attr & FONT_VWF_1BIT) ? vwf_text_bkg_fill : 0;
		uint8_t width = vwf_read_banked_ubyte(vwf_current_font_desc.widths + letter, vwf_current_font_bank);
		uint8_t dx = (8u - vwf_current_offset);
		vwf_current_mask = (0xffu << dx) | (0xffu >> (vwf_current_offset + width));

		vwf_current_rotate = vwf_current_offset;
		vwf_print_shift_char(vwf_tile_data, bitmap, vwf_current_font_bank);
		if ((uint8_t)(vwf_current_offset + width) > 8u) {
			vwf_current_rotate = dx | 0x80u;
			vwf_current_mask = 0xffu >> (width - dx);
			vwf_print_shift_char(vwf_tile_data + 16u, bitmap, vwf_current_font_bank);
		}
		vwf_current_offset += width;

		set_bkg_data(vwf_current_tile, 1, vwf_tile_data);
		if (vwf_current_offset > 7u) {
			memcpy(vwf_tile_data, vwf_tile_data + 16u, 16);
			memset(vwf_tile_data + 16u, vwf_text_bkg_fill, 16);
			vwf_current_offset -= 8u;
			vwf_current_tile++;
			if (vwf_current_offset) set_bkg_data(vwf_current_tile, 1, vwf_tile_data);
			return TRUE;
		} 
		return FALSE;
	} else {
		vwf_set_banked_bkg_data(vwf_current_tile++, 1, bitmap, vwf_current_font_bank);
		vwf_current_offset = 0;
		return TRUE;
	}
}

void vwf_draw_text(uint8_t x, uint8_t y, uint8_t* base_pointer, uint8_t base_tile, const unsigned char * str) {
	static uint8_t * ui_dest_base, *ui_dest_ptr;
	static const uint8_t * ui_text_ptr;
	ui_dest_ptr = ui_dest_base = base_pointer + y * 32 + x;
	ui_text_ptr = str;

	vwf_print_reset(base_tile);
	while (*ui_text_ptr) {
		switch (*ui_text_ptr) {
		case 0x01:
			vwf_activate_font(*++ui_text_ptr);
			break;
		case 0x02:
			ui_dest_ptr = ui_dest_base = (uint8_t *)(0x9C00) + *++ui_text_ptr * 32 + *++ui_text_ptr;
			if (vwf_current_offset) vwf_print_reset(vwf_current_tile + 1u);
			break; 
		case 0x03:
			vwf_text_bkg_fill = *++ui_text_ptr;
			break;
		case '\n':
			ui_dest_ptr = ui_dest_base += 32;
			if (vwf_current_offset) vwf_print_reset(vwf_current_tile + 1u);
			break; 
		default:
			if (vwf_print_render(*ui_text_ptr)) {
				set_vram_byte(ui_dest_ptr++, vwf_current_tile - 1);
			}
			if (vwf_current_offset) set_vram_byte(ui_dest_ptr, vwf_current_tile);
			break;
		}
		ui_text_ptr++;
	}
}

/**
 * Checks the pixel width of a string, inserting a newline at the last space if
 * the width exceeds the max. If no space is found (the word takes up an entire
 * line) no newlines will be inserted.
 * 
 * @param max_pixel_width	The maximum width of the string in pixels.
 * @param str			The string to wrap.
*/
void vwf_wrap_str(uint8_t pixel_width, unsigned char * str)
{
	uint8_t i = 0;
	uint8_t pixels = 0;
	uint8_t last_space = 0;
	while(str[i]) {
		switch (str[i]) {
		case '\n':
			pixels = 0;
			last_space = 0;
			break;
		case ' ':
			last_space = i;
		default:
			pixels += vwf_read_banked_ubyte(
				&vwf_current_font_desc.widths[
					vwf_read_banked_ubyte(
						&vwf_current_font_desc.recode_table[
							(str[i] & ((vwf_current_font_desc.attr & RECODE_7BIT) ? 0x7fu : 0xffu))
						], vwf_current_font_bank
					)
				],
				vwf_current_font_bank
			);
			if (pixels > pixel_width) {
				pixels = 0;
				// Only wrap if there was a previous space.
				if (last_space) {
					str[last_space] = '\n';
					last_space = 0;
				}
			}
			break;
		}
		i++;
	}
}

void vwf_load_font(uint8_t idx, const void * font, uint8_t bank) {
	vwf_fonts[idx].bank = bank;
	vwf_fonts[idx].ptr = (char*) font;
	vwf_activate_font(idx); 
}

void vwf_activate_font(uint8_t idx) {
	vwf_current_font_bank = vwf_fonts[idx].bank;
	vwf_memcpy(&vwf_current_font_desc, vwf_fonts[idx].ptr, sizeof(font_desc_t), vwf_current_font_bank);    
}

uint8_t vwf_next_tile() {
	return (vwf_current_offset) ? vwf_current_tile + 1u : vwf_current_tile;
}