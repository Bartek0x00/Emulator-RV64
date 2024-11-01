#pragma once

#include <vector>
#include "device.hpp"

namespace Emulator {
	class Bus {
	private:
		std::vector<Device&> devices;

	public:
		explicit Bus(void) = default;
		
		Device& find(uint64_t address) const;

		inline void operator+=(Device& device)
		{
			devices.push_back(device);
		}

		inline uint64_t load(uint64_t addr)
		{
			return find(addr).load(addr);
		}

		inline void store(uint64_t addr, uint64_t value)
		{
			find(addr).store(addr);
		}
	};
};
