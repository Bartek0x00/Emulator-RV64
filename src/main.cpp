#include "emulator.hpp"
#include "errors.hpp"

using namespace Emulator;

int main(int argc, char *argv[])
{
    if (argc < 2)
        error("Usage: rv64-emu [image]");

	Emulator::Memory::load_img(argv[1]);
        
	//Emulator::run();

    return 0;
}
