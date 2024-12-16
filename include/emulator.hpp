#pragma once

#include <string_view>
#include "cpu.hpp"
#include "bus.hpp"
#include "mmu.hpp"

namespace Emulator {
	class Bus;
	class Cpu;
	class Mmu;

	std::unique_ptr<Bus> bus;
	std::unique_ptr<Cpu> cpu;
	std::unique_ptr<Mmu> mmu;
};
