#include <gb/gb.h>
#include <gb/incbin.h>

#define DEF_BANK(VARNAME) const void __at(255) __bank_ ## VARNAME;
#define BANK_EXTERN(VARNAME) extern const void __at(255) __bank_ ## VARNAME;

uint8_t banked_get(uint8_t bank, const uint8_t *value) NONBANKED;