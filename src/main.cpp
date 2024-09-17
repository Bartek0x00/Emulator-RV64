#include <stdio.h>
#include <stdlib.h>
#include "emulator.hpp"

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Usage: rv32-emu [image]\n");
		exit(EXIT_FAILURE);
	}

    if (!Emulator::Memory::load_img(argv[1])) {
        fprintf(stderr, "Cannot load %s !\n", argv[1]);
		exit(EXIT_FAILURE);
	}

	Emulator::run();

    return 0;
}
