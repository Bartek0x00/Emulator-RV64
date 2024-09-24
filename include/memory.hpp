#pragma once

#include <cstdint>
#include <vector>
#include "errors.hpp"

namespace Emulator {
	class Memory : public Device {
	private:
		std::vector<uint8_t> mem;
		uint64_t size;
	
	public:
		explicit Memory(uint64_t size);
		Memory(uint64_t size, const char *in_file);

		bool load(uint64_t addr, uint64_t bytes, uint8_t *buffer) override;
		bool store(uint64_t addr, uint64_t bytes, const uint8_t *buffer) override;
		
		inline void reset(void) override
		{
			memset(mem.data(), mem.size(), 0)
		}

		inline uint64_t size(void) const
		{
			return size;
		}
	};
};
