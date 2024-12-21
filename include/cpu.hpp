#pragma once

#include <string_view>
#include "registers.hpp"
#include "interrupt.hpp"
#include "exception.hpp"
#include "bus.hpp"

namespace Emulator {
    class Cpu {
    private:
        enum class Mode : uint64_t {
			USER = 0x00,
			SUPERVISOR = 0x01,
			MACHINE = 0x03,

			INVALID = 0xff
		};
		
		enum class FPURoundingMode : uint64_t {
			ROUND_TO_NEAREST = 0x00,
			ROUND_TO_ZERO = 0x01,
			ROUND_DOWN = 0x02,
			ROUND_UP = 0x03,
			ROUND_NEAREST_MAX_MAGNITUDE = 0x04,
			ROUND_DYNAMIC = 0x05,

			MASK = ROUND_TO_NEAREST | ROUND_TO_ZERO |
				   ROUND_DOWN | ROUND_UP |
				   ROUND_NEAREST_MAX_MAGNITUDE |
				   ROUND_DYNAMIC
		};

	public:
		Interrupt interrupt;
        Exception exception;

        IRegs int_regs;
        FRegs flt_regs;
        CRegs csr_regs;
        uint64_t pc;
		
		std::set<uint64_t> reservations;
        
		explicit Cpu(void);
    };
};
