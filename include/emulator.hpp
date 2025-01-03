#pragma once

#include <string_view>
#include <memory>
#include "cpu.hpp"
#include "bus.hpp"
#include "mmu.hpp"

namespace Emulator {
	class Emulator {
	public:
		explicit Emulator(int argc, char *argv[]);
	};
};
