#pragma once

#include "registers.hpp"
#include "bus.hpp"

namespace Emulator {
    class Cpu {
    private:
      IRegs int_regs;
      FRegs flt_regs;
      CRegs csr_regs;
    
    public:
		explicit Cpu(void);
		
    };
};

