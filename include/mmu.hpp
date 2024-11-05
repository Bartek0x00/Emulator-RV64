#pragma once

#include "bus.hpp"
#include <array>
#include <cstdint>

namespace Emulator {
    class Mmu {
    private:
    public:
        explicit inline Mmu(void);

        inline uint64_t load(uint64_t addr)
        {
            uint64_t p_address = translate(addr, AccessType::Load);

            if (cpu.exc_val != exception::Exception::None)
                return 0;

            return bus.load(cpu, p_address);
        }

        inline void store(uint64_t addr, uint64_t value)
        {
            uint64_t p_address = translate(addr, AccessType::Store);

            if (cpu.exc_val != exception::Exception::None)
                return;

            bus.store(cpu, p_address, value);
        }

        inline uint64_t fetch(uint64_t addr)
        {
            uint64_t p_address = translate(addr, AccessType::Instruction);

            if (cpu.exc_val != exception::Exception::None)
                return 0;

            uint64_t value = bus.load(cpu, p_address);

            if (cpu.exc_val == exception::Exception::LoadAccessFault)
                cpu.exc_val = exception::Exception::InstructionAccessFault;

            return value;
        }
    };
};