#pragma once

#include "registers.hpp"

namespace Emulator {
    class Cpu {
    private:
		Registers<uint64_t, 32> int_regs;
        Registers<double, 32> flt_regs;
        Registers<uint64_t, 4> csr_regs;
    
    public:
		explicit Cpu(void);
		
    };
};

