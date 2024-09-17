#include <stdio.h>

#pragma once

namespace Emulator {
	class Memory {
	private:
		uint8_t mem[SIZE];
	public:
		constexpr size_t SIZE = (1024 * 1024);
		constexpr size_t BASE = 0x80000000;
    	
		static bool load_img(const char *filename);
	
		template<typename U>
		inline U& operator[](uint64_t addr)
		{
			addr -= BASE;

			if (addr < SIZE)
				return reinterpret_cast<U&>(
					this->mem[addr]
				);
			else
				;
		}
	};
};
