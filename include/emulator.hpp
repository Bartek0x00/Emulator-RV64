#pragma once

#include <iostream>
#include <exception>
#include "cpu.hpp"
#include "bus.hpp"

namespace Emulator {

	class Cpu;
	class Bus;
	
	class Emulator {
	private:
		std::unique_ptr<Bus> bus;
		std::unique_ptr<Cpu> cpu;
		
		void run(void);
	
	public:
		explicit Emulator(const char *filepath);
	};
};
