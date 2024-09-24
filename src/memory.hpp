#include <cstdint>
#include <memory>
#include "errors.hpp"

#pragma once

namespace Emulator {
	class Memory {
	public:
		constexpr static uint64_t SIZE = (1024 * 1024 * 1024 * 4ULL);
		constexpr static uint64_t BASE = 0x80000000;
	
	private:
		std::unique_ptr<uint8_t[]> mem;

	public:
		template<typename T>
		inline T& operator[](uint64_t addr)
		{
			addr -= BASE;
			
			if (addr < SIZE)
				return reinterpret_cast<T&>(mem[addr]);
			else
				error<FAIL>(
					"Memory access out of bounds: ",
					addr
				);
		}

		inline operator uint8_t *(void)
		{
			return mem.get();
		}

		static void load_img(const char *filename);
		
		Memory(void) : mem(std::make_unique<uint8_t[]>(SIZE)) {};
	};
};
