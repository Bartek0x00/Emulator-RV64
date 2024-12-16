#pragma once

#include <vector>
#include <string_view>
#include "device.hpp"

namespace Emulator {
	class Bus {
	private:
		std::vector<Device*> devices;

	public:
		explicit Bus(void) = default;
		
		Device& operator[](uint64_t addr) const;
		Device& operator[](std::string_view name) const;
		void dump(void) const;

		inline void add(Device *device)
		{
			devices.push_back(device);
		}
		
		inline uint64_t load(uint64_t addr, uint64_t len)
		{
			return operator[](addr).load(addr, len);
		}

		inline void store(uint64_t addr, uint64_t value, uint64_t len)
		{
			operator[](addr).store(addr, value, len);
		}
	};
};
