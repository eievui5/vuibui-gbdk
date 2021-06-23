#pragma once

/**
 * For some reason GBDK's `SWITCH_ROM_MBC5` sets the 9th bit even when using the
 * 4MiB version. This would be fine normally because I'm sure most users would
 * don't care and when the rom spills over into 8MiB it'll seamlessly work.
 * Cool. Except, that's not what happens because there's also an 8MiB version.
 * Why!? The 4MiB version ALWAYS sets bit 9 to zero, without considering the
 * argument. There is NO reason for these to be seperate in the first place, but
 * if they're going to be seperate , at least optimize the 4MiB version!!!!
 * 
 * Ugh. This way I get a more optimal version, and if I somehow need 8MiB I can
 * just edit this.
*/
#define SET_BANK(bank) _current_bank = (bank), *(unsigned char *)0x2000 = (bank)