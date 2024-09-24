#include "memory.hpp"
#include "registers.hpp"

#pragma once

namespace Emulator {
	class Memory;
	class Registers;
	//class Cpu;

	extern Memory memory;
	extern Registers registers;
	//extern Cpu cpu;

	void run(void);

	extern bool RVC;
};
