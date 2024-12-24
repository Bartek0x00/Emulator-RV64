#pragma once

#include <string_view>
#include <memory>
#include "cpu.hpp"
#include "bus.hpp"
#include "mmu.hpp"

namespace Emulator {
	class Emulator {
	public:
		std::unique_ptr<Cpu> cpu;
		std::unique_ptr<Mmu> mmu;
		std::unique_ptr<Bus> bus;

		explicit Emulator(int argc, char *argv[]);
	};
};
