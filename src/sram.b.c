#include "include/save.h"

const char test_string[] = "This is a Vuiiger engine save file. Do not edit this corruption test string!";
char sram_corruption_check[sizeof(test_string)];

save_file sram_file_1;