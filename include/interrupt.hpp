#pragma once

#include <string_view>
#include <cstdint>

namespace Emulator {
    class Interrupt {
    public:
        enum InterruptValue : uint64_t {
            NONE = (~0ULL),
            USER_SOFTWARE = 0,
            SUPERVISOR_SOFTWARE = 1,
            MACHINE_SOFTWARE = 3,
            USER_TIMER = 4,
            SUPERVISOR_TIMER = 5,
            MACHINE_TIMER = 7,
            USER_EXTERNAL = 8,
            SUPERVISOR_EXTERNAL = 9,
            MACHINE_EXTERNAL = 11,
        };
		
		InterruptValue current;

        std::string_view get_name(void);
        void get_pending(void);
        void process(void);

        explicit constexpr inline Interrupt(void) : 
			current(NONE) {};
    };
};
