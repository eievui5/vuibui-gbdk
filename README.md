# vuibui-gbdk

To build, run `make` in the root directory.

## Dependancies
 - RGBDS 0.5.2
 - GBDK 2020 (Must compile the current master branch as of July 2021)
 - GNU Make 4.3
### Windows:
 - A Unix environment, such as MSYS2 or WSL.
### Linux:
 - WINE

## Makefile

The makefile compiles `.c`, `.s`, and `.png` files to the `obj/` and `res/`
directories.

`src/` is set as an include path; all includes should be relative to it.

### Special Rules
 - `.b.c` files will be compiled using SRAM for global variables rather than WRAM.
 - `.h.png` files will be compiled as 8x16 rather than 8x8.
