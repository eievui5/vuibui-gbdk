################################################
#                                              #
#             PROJECT CONFIGURATION            #
#                                              #
################################################

# Change this to the name of your rom.
ROMNAME = vuibui-gbdk
# Change this to "gbc" if your rom is for the Game Boy Color only.
ROMEXT  = gb

# Your ROM's 15-character Title. This must be within quotes.
TITLE = "VUIBUI-GBDK"

# System Compatability Flag.
# Leave Blank for DMG-only
# -Wm-yc : Game Boy Color compatible
# -Wm-yC : Game Boy Color only
# -Wm-ys : Super Game Boy compatible
COMPAT = 

# MBC type, tells which hardware is in the cart
# See https://gbdev.io/pandocs/#_0147-cartridge-type for a list of MBC types
MBCTYPE := 0x1B
# 0x1B is MBC5 with SRAM + Battery

# The number of additional ROM banks to include on the cartridge. Each bank is
# equivalent to 16KiBs of ROM. This should be a power of 2.
ROMBANKS := A

# The number of external RAM banks in include on the cartidge. Each bank is
# equivalent to 8KiBs of RAM. This should be a power of 2
RAMBANKS := 1

# List of C source directories.
# Add a new $(wildcard $(SRCDIR)/<directory>*.c) when more directories are needed.
CSRC 	:= $(wildcard $(SRCDIR)/*.c)

# List of ASM source directories.
# Add a new $(wildcard $(SRCDIR)/<directory>*.s) when more directories are needed.
SSRC	:= $(wildcard $(SRCDIR)/*.s)

# List of binary source directories. These files will be converted to C sources
# and headers to be included in the program.
# Add a new $(wildcard $(SRCDIR)/<directory>*.bin) when more directories are needed.
BINSRC	:= 

# List of PNG source directories. These files will be converted to C sources
# and headers to be included in the program.
# Add a new $(wildcard $(SRCDIR)/<directory>*.png) when more directories are needed.
GFXSRC	:= $(wildcard $(SRCDIR)/gfx/tiles/*.png)

# List of tall PNG source directories, for 8x16 sprites. These files will be 
# converted to C sources and headers to be included in the program.
# Add a new $(wildcard $(SRCDIR)/<directory>*.png) when more directories are needed.
TALLSRC := $(wildcard $(SRCDIR)/gfx/sprites/*.png)