#include <stdint.h>
#include "conditions.hpp"

#pragma once

using namespace Emulator;

class Registers {
private:
    static uint32_t regs[COUNT];
public:
    enum {
        X0  = 0b000000,
        X1  = 0b000001,
        X2  = 0b000010,
        X3  = 0b000011,
        X4  = 0b000100,
        X5  = 0b000101,
        X6  = 0b000110,
        X7  = 0b000111,
        X8  = 0b001000,
        X9  = 0b001001,
        X10 = 0b001010,
        X11 = 0b001011,
        X12 = 0b001100,
        X13 = 0b001101,
        X14 = 0b001110,
        X15 = 0b001111,
        X16 = 0b010000,
        X17 = 0b010001,
        X18 = 0b010010,
        X19 = 0b010011,
        X20 = 0b010100,
        X21 = 0b010101,
        X22 = 0b010110,
        X23 = 0b010111,
        X24 = 0b011000,
        X25 = 0b011001,
        X26 = 0b011010,
        X27 = 0b011011,
        X28 = 0b011100,
        X29 = 0b011101,
        X30 = 0b011110,
        X31 = 0b011111,
        PC  = 0b100000,
    };

    static constexpr uint32_t COUNT = 33;

    static inline uint32_t& operator[]
};
