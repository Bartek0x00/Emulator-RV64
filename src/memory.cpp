#include "registers.hpp"
#include "memory.hpp"

using namespace Emulator;

Memory::load_img(const char *filename)
{
    FILE *file = fopen(filename, "r+");
    bool ret = true;
    
    // TODO: Handling image encoding

    fclose(file);
}

Memory::next_instr(void)
{
    return Registers[Registers::PC]++;
}