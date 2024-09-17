#pragma once

namespace Emulator {
    class Bus {
    private:
        Memory mem();
    
    public:
        template <typename U>
        inline U& operator[](uint64_t addr)
        {
            mem<U>[addr];
        }
    };
};