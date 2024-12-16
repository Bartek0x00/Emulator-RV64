#pragma once

#include <string_view>

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
            MACHINE_TIME = 7,
            USER_EXTERNAL = 8,
            SUPERVISOR_EXTERNAL = 9,
            MACHINE_EXTERNAL = 11,
        };

        std::string_view get_name(InterruptValue int_value);
        InterruptValue get_pending(void);
        void process(InterruptValue int_value);

        explicit inline Interrupt(void) = default;
    };
};
