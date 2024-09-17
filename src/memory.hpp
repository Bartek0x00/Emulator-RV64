#include <stdio.h>

#pragma once

namespace Emulator {
	class Memory {
	private:
		uint8_t mem[SIZE] = {0};

	public:
		constexpr size_t SIZE = (1024 * 1024);
		constexpr size_t BASE = 0x80000000;
	
		template<typename U>
		inline U& operator[](uint64_t addr)
		{
			addr -= BASE;

			if (addr < SIZE)
				return reinterpret_cast<U&>(
					this->mem[addr]
				);
			else
				error<FAIL>("Memory access out of bounds");
		}

		inline operator uint16_t *(void)
		{
			return &this->mem;
		}
	};
};
