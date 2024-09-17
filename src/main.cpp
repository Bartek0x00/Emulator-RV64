#include <stdio.h>
#include <stdlib.h>
#include "emulator.hpp"

int main(int argc, char *argv[])
{
    if (argc < 2)
        error<FAIL>("Usage: rv64-emu [image]");

    if (!Emulator::Memory::load_img(argv[1]));
        error<FAIL>("Cannot load the image");
        
	Emulator::run();

    return 0;
}
