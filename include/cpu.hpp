#pragma once

#include <string_view>
#include "registers.hpp"
#include "interrupt.hpp"
#include "exception.hpp"
#include "bus.hpp"

namespace Emulator {
    class Cpu {
    private:
        Interrupt interrupt;
        Exception exception;

        IRegs int_regs;
        FRegs flt_regs;
        CRegs csr_regs;
    
    public:
		explicit Cpu(void);
    };
};
