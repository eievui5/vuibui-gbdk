        .globl _vwf_current_rotate, _vwf_current_mask, _vwf_inverse_map
        .globl _memcpy
        .globl _set_bkg_data

        .area _BSS
        
__save: 
        .ds 0x01 

        .area _CODE

; void vwf_memcpy(void* to, const void* from, size_t n, UBYTE bank);
_vwf_memcpy::
        ldh a, (__current_bank)
        ld  (#__save),a

        ldhl sp, #8
        ld  a, (hl)
        ldh (__current_bank),a
        ld  (#0x2000), a

        pop bc
        call _memcpy

        ld  a, (#__save)
        ldh (__current_bank),a
        ld  (#0x2000), a
        ld  h, b
        ld  l, c
        jp  (hl)

; UBYTE vwf_read_banked_ubyte(const void * src, UBYTE bank) __preserves_regs(b, c); 
_vwf_read_banked_ubyte::
        ldh a, (__current_bank)
        ld  (#__save),a

        ldhl  sp, #4
        ld  a, (hl-)
        ldh (__current_bank),a
        ld  (#0x2000), a

        ld  a, (hl-)
        ld  l, (hl)
        ld  h, a
        ld  e, (hl)

        ld  a, (#__save)
        ldh (__current_bank),a
        ld  (#0x2000), a
        ret

; void vwf_set_banked_bkg_data(UBYTE i, UBYTE l, const unsigned char* ptr, UBYTE bank);
_vwf_set_banked_bkg_data::
        ldh a, (__current_bank)
        ld  (#__save),a

        ldhl  sp, #6
        ld  a, (hl)
        ldh (__current_bank),a
        ld  (#0x2000), a

        pop bc
        call  _set_bkg_data

        ld  a, (#__save)
        ldh (__current_bank),a
        ld  (#0x2000), a
        ld  h, b
        ld  l, c
        jp  (hl)

; void vwf_set_banked_win_data(UBYTE i, UBYTE l, const unsigned char* ptr, UBYTE bank);
_vwf_set_banked_win_data::
        ldh a, (__current_bank)
        ld  (#__save),a

        ldhl  sp, #6
        ld  a, (hl)
        ldh (__current_bank),a
        ld  (#0x2000), a

        pop bc
        call  _set_win_data

        ld  a, (#__save)
        ldh (__current_bank),a
        ld  (#0x2000), a
        ld  h, b
        ld  l, c
        jp  (hl)

; void vwf_print_shift_char(void * dest, const void * src, UBYTE bank);
_vwf_print_shift_char::
        ldhl sp, #6
        
        ldh a, (__current_bank)
        push af
        ld a, (hl-)
        ldh (__current_bank), a
        ld  (#0x2000), a

        ld a, (hl-)
        ld d, a
        ld a, (hl-)
        ld e, a 
        ld a, (hl-)
        ld l, (hl)
        ld h, a 

        ld a, #8
3$:
        push af

        ld a, (de)
        ld c, a
        ld a, (_vwf_inverse_map)
        xor c
        ld c, a
        inc de
                
        ld a, (de)
        ld b, a
        ld a, (_vwf_inverse_map)
        xor b
        ld b, a
        inc de

        ld a, (_vwf_current_rotate)
        sla a
        jr z, 1$
        jr c, 4$
        srl a
        srl a
        jr nc, 6$
        srl c
        srl b
6$:
        or a
        jr z, 1$
2$:
        srl c
        srl b
        srl c
        srl b
        dec a
        jr nz, 2$
        jr 1$
4$:
        srl a
        srl a
        jr nc, 7$
        sla c
        sla b
7$:     or a
        jr z, 1$
5$:
        sla c
        sla b
        sla c
        sla b
        dec a
        jr nz, 5$
1$:
        ld a, (_vwf_current_mask)
        and (hl)
        or c
        ld (hl+), a

        ld a, (_vwf_current_mask)
        and (hl)
        or b
        ld (hl+), a

        pop af
        dec a
        jr nz, 3$

        pop af
        ldh (__current_bank),a
        ld  (#0x2000), a

        ret