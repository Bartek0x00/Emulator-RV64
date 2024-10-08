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
	
		bool RVC = true;

		void run(void);
	
	public:
		explicit Emulator(const char *filepath);
	};
};