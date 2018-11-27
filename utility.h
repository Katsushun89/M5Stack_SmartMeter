#pragma once
#include <M5Stack.h>

bool isOverWaitTime(uint32_t start_time, uint32_t cur_time, uint32_t wait_time);
unsigned int hexToDec(String hex_string);
