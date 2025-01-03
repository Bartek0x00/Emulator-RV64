#pragma once

#include <string_view>
#include <cstdint>

namespace Emulator {
    class Exception {
    public:
        enum ExceptionValue : uint64_t {
            NONE = (~0ULL),
            INSTRUCTION_ADDRESS_MISALIGNED = 0,
            INSTRUCTION_ACCESS_FAULT = 1,
            ILLEGAL_INSTRUCTION = 2,
            BREAKPOINT = 3,
            LOAD_ADDRESS_MISALIGNED = 4,
            LOAD_ACCESS_FAULT = 5,
            STORE_ADDRESS_MISALIGNED = 6,
            STORE_ACCESS_FAULT = 7,
            ECALL_UMODE = 8,
            ECALL_SMODE = 9,
            ECALL_MMODE = 11,
            INSTRUCTION_PAGE_FAULT = 12,
            LOAD_PAGE_FAULT = 13,
            STORE_PAGE_FAULT = 15,
        };

		ExceptionValue current;
		uint64_t value;

        std::string_view get_name(void);
        void process(void);

        explicit constexpr inline Exception(void) :
			current(NONE), value(0) {};
    };
};
