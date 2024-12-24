#include "emulator.hpp"

using namespace Emulator;

int main(int argc, char *argv[])
{
	try
		Emulator(argc, argv);
	catch (...)
		return 0;		
	 
	return 0;
}
