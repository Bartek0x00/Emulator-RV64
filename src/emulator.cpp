#include "emulator.hpp"

namespace Emulator {
    Cpu cpu;
};

using namespace Emulator;

void run(void)
{
    while (1)
        cpu(memory<uint64_t>[registers[Registers::PC]++]);
}