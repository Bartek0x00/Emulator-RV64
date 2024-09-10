#include <stdio.h>

#pragma once

using namespace Emulator;

class Memory {
public:
    static bool load_img(const char *filename);
    static inline void next_instr(void);
};