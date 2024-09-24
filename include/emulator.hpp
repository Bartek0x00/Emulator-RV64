#pragma once

#include "cpu.hpp"
#include "bus.hpp"

namespace Emulator {
	class Bus;
	class Cpu;

	extern Bus bus;
	extern Cpu cpu;

	void run(void);

	extern bool RVC;
};
