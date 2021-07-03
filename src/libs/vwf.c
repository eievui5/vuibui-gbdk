#include <gb/gb.h>
#include <string.h>

#include "include/int.h"
#include "vwf.h"

vwf_farptr_t vwf_fonts[4];

static u8 vwf_current_offset;
static u8 vwf_tile_data[16 * 2];
u8 vwf_current_mask;
u8 vwf_current_rotate;
u8 vwf_inverse_map;
u8 vwf_current_tile;
u8 vwf_text_bkg_fill = 0;

font_desc_t vwf_current_font_desc;
u8 vwf_current_font_bank;

void vwf_print_shift_char(void * dest, const void * src, u8 bank);
void vwf_memcpy(void* to, const void* from, size_t n, u8 bank);
u8 vwf_read_banked_ubyte(const void * src, u8 bank) __preserves_regs(b, c);
void vwf_set_banked_bkg_data(u8 i, u8 l, const unsigned char* ptr, u8 bank);
void vwf_set_banked_win_data(u8 i, u8 l, const unsigned char* ptr, u8 bank);

void vwf_print_reset(u8 tile) {
	vwf_current_tile = tile;
	vwf_current_offset = 0;
	memset(vwf_tile_data, vwf_text_bkg_fill, sizeof(vwf_tile_data));
}

u8 vwf_print_render(const unsigned char ch) {
	u8 letter = vwf_read_banked_ubyte(vwf_current_font_desc.recode_table + (ch & ((vwf_current_font_desc.attr & RECODE_7BIT) ? 0x7fu : 0xffu)), vwf_current_font_bank);
	const u8 * bitmap = vwf_current_font_desc.bitmaps + letter * 16u;
	if (vwf_current_font_desc.attr & FONT_VWF) {
		vwf_inverse_map = (vwf_current_font_desc.attr & FONT_VWF_1BIT) ? vwf_text_bkg_fill : 0;
		u8 width = vwf_read_banked_ubyte(vwf_current_font_desc.widths + letter, vwf_current_font_bank);
		u8 dx = (8u - vwf_current_offset);
		vwf_current_mask = (0xffu << dx) | (0xffu >> (vwf_current_offset + width));

		vwf_current_rotate = vwf_current_offset;
		vwf_print_shift_char(vwf_tile_data, bitmap, vwf_current_font_bank);
		if ((u8)(vwf_current_offset + width) > 8u) {
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

void vwf_draw_text(u8 x, u8 y, u8 base_tile, const unsigned char * str) {
	static u8 * ui_dest_base, *ui_dest_ptr;
	static const u8 * ui_text_ptr;
	ui_dest_ptr = ui_dest_base = (u8 *)(0x9C00) + y * 32 + x;
	ui_text_ptr = str;

	vwf_print_reset(base_tile);
	while (*ui_text_ptr) {
		switch (*ui_text_ptr) {
		case 0x01:
			vwf_activate_font(*++ui_text_ptr);
			break;
		case 0x02:
			ui_dest_ptr = ui_dest_base = (u8 *)(0x9C00) + *++ui_text_ptr * 32 + *++ui_text_ptr;
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
void vwf_wrap_str(u8 pixel_width, unsigned char * str)
{
	u8 i = 0;
	u8 pixels = 0;
	u8 last_space = 0;
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

void vwf_load_font(u8 idx, const void * font, u8 bank) {
	vwf_fonts[idx].bank = bank;
	vwf_fonts[idx].ptr = (void *)font;
	vwf_activate_font(idx); 
}

void vwf_activate_font(u8 idx) {
	vwf_current_font_bank = vwf_fonts[idx].bank;
	vwf_memcpy(&vwf_current_font_desc, vwf_fonts[idx].ptr, sizeof(font_desc_t), vwf_current_font_bank);    
}

u8 vwf_next_tile() {
	return (vwf_current_offset) ? vwf_current_tile + 1u : vwf_current_tile;
}