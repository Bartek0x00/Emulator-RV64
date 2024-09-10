#include <stdio.h>
#include <stdlib.h>
#include "Emulator.hpp"

using namespace Emulator;

int main(int argc, char *argv[])
{
    if (argc < 2)
        error("Usage: rv32-emu [image]");

    if (!Memory::load_img(argv[1]))
        error("Cannot load the provided image!");

    while(1) {
        if (!Operations(Memory::next_instr()))
            break;
    }

    return 0;
}