#pragma once

#include <cstdint>
#include <vector>
#include "errors.hpp"

namespace Emulator {
	class Dram : public Device {
	private:
		std::vector<uint8_t> mem;
		uint64_t size;
	
	public:
		explicit Memory(uint64_t size, char *input_file);

		bool load(uint64_t addr, uint64_t bytes, uint8_t *buffer) override;
		bool store(uint64_t addr, uint64_t bytes, const uint8_t *buffer) override;
		
		inline void reset(void) override
		{
			memset(mem.data(), 0, mem.size());
		}

		inline uint64_t size(void) const
		{
			return size;
		}
	};
};
