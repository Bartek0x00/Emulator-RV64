#include "emulator.hpp"

int main(int argc, char *argv[])
{
	try {
		Emulator::Emulator(argc, argv);
	} catch (...) {
		return 0;
	}		
	 
	return 0;
}
