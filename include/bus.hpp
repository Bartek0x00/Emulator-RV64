#pragma once

#include <vector>
#include "device.hpp"

namespace Emulator {
	class Bus {
	private:
		std::vector<Device&> devices;

	public:
		explicit Bus(void) = default;

		inline void operator+=(Device& device)
		{
			devices.push_back(device);
		}

		Device& operator[](uint64_t address) const;
	};
};
