#pragma once

extern uint8_t dialogue_bank;
extern const char* dialogue_string;

void show_dialogue() NONBANKED;
void print_dialogue() BANKED;