#include <algorithm>
#include "bus.hpp"
#include "errors.hpp"

Bus::Device& operator[](uint64_t address) const
{
	for (const Device& device : devices) {
		if (address >= device.base && \
			address < device.base + device.size)
		{
			return device;
		}
	}

	error<FAIL>(
		"Cannot find the device at address: ", 
		address
	);

	return nullptr;
}
