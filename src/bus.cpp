#include <algorithm>
#include "bus.hpp"
#include "errors.hpp"

Device& Bus::find(uint64_t addr) const
{
	for (const Device& device : devices) {
		if (addr >= device.base && \
			addr < device.base + device.size)
		{
			return device;
		}
	}

	error<FAIL>(
		"Cannot find the device at address: ", 
		addr
	);

	return nullptr;
}

